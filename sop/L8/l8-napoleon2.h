#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define STACK_SIZE 16
#define MAXBUF 512
#define ADJUTANTS_COUNT 4

// --- ZASOBY WSPÓŁDZIELONE (STOS) ---
char message_stack[STACK_SIZE][MAXBUF];
int stack_top = 0; // Wskazuje na pierwsze WOLNE miejsce na stosie (0 oznacza pusty stos)

// --- MECHANIZMY SYNCHRONIZACJI ---
pthread_mutex_t stack_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stack_cond = PTHREAD_COND_INITIALIZER;

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig) {
    do_work = 0;
}

// =========================================================
// WĄTEK ADIUTANTA (KONSUMENT)
// =========================================================
void* adjutant_thread(void* arg) {
    int id = *((int*)arg);
    free(arg);
    char local_buf[MAXBUF];

    while (1) {
        // 1. Zablokowanie dostępu do stosu
        pthread_mutex_lock(&stack_mutex);

        // 2. Oczekiwanie na wiadomości (Zabezpieczenie przed aktywnym czekaniem)
        while (stack_top == 0 && do_work) {
            pthread_cond_wait(&stack_cond, &stack_mutex);
        }

        // 3. Sprawdzenie, czy serwer kończy pracę i stos jest pusty
        if (!do_work && stack_top == 0) {
            pthread_mutex_unlock(&stack_mutex);
            break;
        }

        // 4. Pobranie wiadomości ze szczytu stosu (LIFO)
        stack_top--;
        strcpy(local_buf, message_stack[stack_top]);

        // 5. Odblokowanie stosu dla innych (parsowanie robimy POZA sekcją krytyczną!)
        pthread_mutex_unlock(&stack_mutex);

        // 6. Parsowanie i wypisywanie komunikatu (Z etapu 1)
        int x, y, p;
        char name[129];
        memset(name, 0, sizeof(name));

        int parsed = sscanf(local_buf, "%d %d %d %[^\n]", &x, &y, &p, name);

        if (parsed != 4 || x < 0 || x > 99 || y < 0 || y > 99 || (p != 0 && p != 1)) {
            fprintf(stderr, "[Adiutant %d] Odrzucono: Zle sformatowany meldunek.\n", id);
        } else {
            if (p == 1) {
                printf("[Adiutant %d] Nasz oddzial %s byl widziany na pozycji %d:%d\n", id, name, x, y);
            } else {
                printf("[Adiutant %d] Wrogi oddzial %s byl widziany na pozycji %d:%d\n", id, name, x, y);
            }
        }
    }

    return NULL;
}

// =========================================================
// GŁÓWNY WĄTEK (PRODUCENT UDP)
// =========================================================
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Uzycie: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Obsługa sygnału zamykającego
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, NULL);

    // Utworzenie gniazda UDP
    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return EXIT_FAILURE;
    }

    // Uruchomienie puli wątków adiutantów
    pthread_t adjutants[ADJUTANTS_COUNT];
    for (int i = 0; i < ADJUTANTS_COUNT; i++) {
        int* id;
        id = (int*)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&adjutants[i], NULL, adjutant_thread, id);
    }

    printf("Sztab Napoleona otwarty. Oczekuje na meldunki na porcie %s...\n", argv[1]);

    char recv_buf[MAXBUF];
    while (do_work) {
        socklen_t size = sizeof(addr);
        ssize_t receivedBytes = recvfrom(fd, recv_buf, MAXBUF - 1, 0, (struct sockaddr*)&addr, &size);
        
        if (receivedBytes < 0) {
            if (errno == EINTR) continue; // Przerwanie sygnałem SIGINT
            perror("recvfrom");
            continue;
        }

        recv_buf[receivedBytes] = '\0'; // Zabezpieczenie końca stringa

        // --- SEKCJA KRYTYCZNA: DODANIE NA STOS ---
        pthread_mutex_lock(&stack_mutex);

        if (stack_top < STACK_SIZE) {
            // Skopiowanie wiadomości na stos
            strcpy(message_stack[stack_top], recv_buf);
            stack_top++;

            // Wybudzenie jednego uśpionego adiutanta
            pthread_cond_signal(&stack_cond);
        } else {
            fprintf(stderr, "[Sztab] Stos pełny! Meldunek przepadł w chaosie bitwy.\n");
        }

        pthread_mutex_unlock(&stack_mutex);
    }

    // --- PROCEDURA EWAKUACJI SZTABU ---
    printf("\nEwakuacja sztabu! Konczenie pracy...\n");
    
    // Budzimy wszystkich uśpionych adiutantów, żeby zauważyli do_work == 0 i zakończyli działanie
    pthread_cond_broadcast(&stack_cond);

    for (int i = 0; i < ADJUTANTS_COUNT; i++) {
        pthread_join(adjutants[i], NULL);
    }

    close(fd);
    pthread_mutex_destroy(&stack_mutex);
    pthread_cond_destroy(&stack_cond);
    
    printf("Sztab zamkniety.\n");
    return EXIT_SUCCESS;
}