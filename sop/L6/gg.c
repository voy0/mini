#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define SHARED_MEM_NAME "/sop-uczta-mem"
#define MAX_HOUSES 5

typedef struct {
    int panic;
    pthread_mutex_t mutexes[MAX_HOUSES];
    pthread_cond_t conds[MAX_HOUSES];
    int occupancy[MAX_HOUSES];
} SharedData;

void usage(const char* name) {
    fprintf(stderr, "USAGE: %s N M C\n", name);
    fprintf(stderr, "N - liczba szlachcicow (10-30)\n");
    fprintf(stderr, "M - liczba gospod (2-5)\n");
    fprintf(stderr, "C - pojemnosc gospody (2-4)\n");
    exit(EXIT_FAILURE);
}

void ms_sleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, &ts);
}

void set_timeout(struct timespec *ts, int ms) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_nsec += ms * 1000000;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_sec += ts->tv_nsec / 1000000000;
        ts->tv_nsec = ts->tv_nsec % 1000000000;
    }
}

void noble_work(int m, int c, SharedData* data) {
    srand(getpid() ^ time(NULL));

    for (int i = 0; i < 3; i++) { // Każdy szlachcic chce odwiedzić 3 gospody
        if (data->panic) break; // Uciekamy, jeśli jest panika w królestwie

        int dom = rand() % m;
        
        int err = pthread_mutex_lock(&data->mutexes[dom]);
        if (err == EOWNERDEAD) {
            printf("Szlachcic %d: Na litosc boska, ktos lezy w progu gospody %d! Uciekam!\n", getpid(), dom);
            data->panic = 1;
            pthread_mutex_consistent(&data->mutexes[dom]);
            pthread_mutex_unlock(&data->mutexes[dom]);
            break;
        } else if (err != 0) ERR("pthread_mutex_lock");

        // Gospoda pełna - czekamy z limitem czasu!
        int offended = 0;
        while (data->occupancy[dom] >= c) {
            struct timespec ts;
            set_timeout(&ts, 500); // 500 ms cierpliwości szlachcica
            
            // Magia zmiennej warunkowej - usypia nas i ZWALNIA mutex. Gdy nas obudzi, ZABLOKUJE go z powrotem.
            int res = pthread_cond_timedwait(&data->conds[dom], &data->mutexes[dom], &ts);
            
            if (res == ETIMEDOUT) {
                printf("Szlachcic %d: Skandal! Gospoda %d jest pelna, wracam do siebie!\n", getpid(), dom);
                offended = 1;
                break; // Wychodzimy z pętli oczekiwania
            } else if (res == EOWNERDEAD) {
                data->panic = 1; // Prawdopodobieństwo śmierci przy cond_wait, ubezpieczamy się
                break;
            } else if (res != 0) {
                ERR("pthread_cond_timedwait");
            }
        }

        if (data->panic) {
            pthread_mutex_unlock(&data->mutexes[dom]);
            break;
        }

        if (offended) {
            pthread_mutex_unlock(&data->mutexes[dom]);
            exit(EXIT_SUCCESS); // Prawdziwy foch - koniec zabawy
        }

        // --- BIESIADA ---
        data->occupancy[dom]++;
        printf("Szlachcic %d: Wchodze do gospody %d (Zajete: %d/%d)\n", getpid(), dom, data->occupancy[dom], c);
        pthread_mutex_unlock(&data->mutexes[dom]); // Wszedł, nie blokuje drzwi!

        ms_sleep(300 + rand() % 300); // Biesiada od 300ms do 600ms

        // Szansa na zgon po biesiadzie
        if (rand() % 100 < 3) {
            printf("Szlachcic %d: O nie... to mieso bylo nieswieze...\n", getpid());
            pthread_mutex_lock(&data->mutexes[dom]);
            // Umiera trzymając zamek gospody
            abort(); 
        }

        // --- WYJŚCIE Z GOSPODY ---
        err = pthread_mutex_lock(&data->mutexes[dom]);
        if (err == EOWNERDEAD) {
            printf("Szlachcic %d: Ktos trupem padl wychodzac! Alarm!\n", getpid());
            data->panic = 1;
            pthread_mutex_consistent(&data->mutexes[dom]);
            pthread_mutex_unlock(&data->mutexes[dom]);
            break;
        } else if (err != 0) ERR("pthread_mutex_lock");

        data->occupancy[dom]--;
        printf("Szlachcic %d: Opuszczam gospode %d. (Zajete: %d/%d)\n", getpid(), dom, data->occupancy[dom], c);
        
        // Informujemy resztę kolejki: ZWOLNIŁO SIĘ MIEJSCE!
        pthread_cond_signal(&data->conds[dom]);
        
        pthread_mutex_unlock(&data->mutexes[dom]);
        
        ms_sleep(100); // Spacer przed kolejną gospodą
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    if (argc != 4) usage(argv[0]);

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int c = atoi(argv[3]);

    if (n < 10 || n > 30 || m < 2 || m > 5 || c < 2 || c > 4) usage(argv[0]);

    // Oczyszczanie przed startem
    shm_unlink(SHARED_MEM_NAME);

    // ==