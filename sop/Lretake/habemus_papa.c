#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#define MIN_N 1
#define MAX_N 15
#define MIN_M 5
#define MAX_M 20

#define SEMAPHORE_NAME "/gate"
#define SHM_NAME "/houses"

#define ERR(source)                                     \
    do                                                  \
    {                                                   \
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); \
        perror(source);                                 \
        kill(0, SIGKILL);                               \
        exit(EXIT_FAILURE);                             \
    } while (0)

void usage(char* program_name)
{
    fprintf(stderr, "Usage: \n");
    fprintf(stderr, "\t%s n m\n", program_name);
    fprintf(stderr, "\t  n - number of townhouses, %d <= n <= %d\n", MIN_N, MAX_N);
    fprintf(stderr, "\t  m - number of nobles, %d <= m <= %d\n", MIN_M, MAX_M);
    exit(EXIT_FAILURE);
}

void ms_sleep(unsigned int milli)
{
    time_t sec = (int)(milli / 1000);
    milli = milli - (sec * 1000);
    struct timespec ts = {0};
    ts.tv_sec = sec;
    ts.tv_nsec = milli * 1000000L;
    if (nanosleep(&ts, &ts))
        ERR("nanosleep");
}

void safe_lock(pthread_mutex_t *mutex) {
  int err = pthread_mutex_lock(mutex);
  if (err != 0) {
    if (err == EOWNERDEAD) {
      pthread_mutex_consistent(mutex);
    } else {
      ERR("pthread_mutex_lock");
    }
  }
}

typedef struct{
    pthread_mutex_t mutex[MAX_N];
    int free_houses;
    pthread_mutex_t fh_mutex;
    pthread_cond_t fh_cond;
}kwatery_t;

int main(int argc, char** argv) 
{ 
    char* name = argv[0];
    if(argc != 3){
        usage(name);
    }
    int n = atoi(argv[1]); // kamienice
    int m = atoi(argv[2]); // szlachice

    if(n < MIN_N || n > MAX_N || m < MIN_M || m > MAX_M){
        usage(name);
    }

    kwatery_t* kwatery = (kwatery_t*)mmap(NULL, sizeof(kwatery_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(kwatery == MAP_FAILED){
        ERR("mmap");
    }
    
    int* houses;
    ssize_t size = sizeof(int)*n;

    shm_unlink(SHM_NAME);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if(shm_fd < 0){
        ERR("shm_open");
    }

    if (ftruncate(shm_fd, size) == -1)
            ERR("ftruncate");

    houses = (int*)mmap(NULL, size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, shm_fd, 0);
    if (houses == MAP_FAILED)
        ERR("mmap");

    for(int i = 0; i < n; i++){
        houses[i] = 0;
    }
    munmap(houses, size);
    close(shm_fd);

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);
    for(int i = 0; i < n; i++){
        pthread_mutex_init(&kwatery->mutex[i], &mutex_attr);
    }
    pthread_mutex_init(&kwatery->fh_mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&kwatery->fh_cond, &cond_attr);

    kwatery->free_houses = n;

    for(int i = 0; i < m; i++){
        int f = fork();
        switch(f){
            case 0:{
                srand(getpid());
                shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
                if(shm_fd < 0){
                    ERR("shm_open");
                }

                houses = (int*)mmap(NULL, size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, shm_fd, 0);
                if (houses == MAP_FAILED)
                    ERR("mmap");


                int found_house = 0;
                while(found_house == 0){
                    for(int j = 0; j < n; j++){
                        int err = pthread_mutex_trylock(&kwatery->mutex[j]);
                        if(err == 0){
                            printf("[%d] Jakże iż komfortowy dom numeru %d\n", getpid(), j);

                            safe_lock(&kwatery->fh_mutex);
                            kwatery->free_houses--;
                            pthread_mutex_unlock(&kwatery->fh_mutex);

                            ms_sleep(300);

                            if(rand()% 10 <= 1)
                            {
                                printf("[%d] Zniszczylem dom %d\n", getpid(), j);
                                houses[j] ++; 
                            }

                            safe_lock(&kwatery->fh_mutex);
                            kwatery->free_houses++;
                            pthread_mutex_unlock(&kwatery->fh_mutex);


                            pthread_mutex_unlock(&kwatery->mutex[j]);
                            found_house = 1;
                            pthread_cond_signal(&kwatery->fh_cond);
                            break;
                        }
                        if (err != 0) {
                            if (err == EOWNERDEAD) {
                                pthread_mutex_consistent(&kwatery->mutex[j]);
                            } 
                            else if(err == EBUSY){
                                continue;
                            }
                            else{
                                ERR("pthread_mutex_lock");
                            }
                        }
                    }
                    if(found_house == 0){
                        safe_lock(&kwatery->fh_mutex);
                        while(kwatery->free_houses == 0){
                            pthread_cond_wait(&kwatery->fh_cond, &kwatery->fh_mutex);              
                        }
                        pthread_mutex_unlock(&kwatery->fh_mutex);
                        printf("[%d] Sproboje jeszcze raz\n", getpid());
                    }
                }
                close(shm_fd);
                munmap(houses, size);
                munmap(kwatery, sizeof(kwatery_t));
                exit(EXIT_SUCCESS);
            }
            case -1:
                ERR("fork");
                break;
            
        }
    }
    for(int i = 0; i < m; i++){
        wait(NULL);
    }
    for(int i = 0; i < n; i++){
        pthread_mutex_destroy(&kwatery->mutex[i]);
    }
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if(shm_fd < 0){
        ERR("shm_open");
    }

    houses = (int*)mmap(NULL, size, PROT_READ | PROT_WRITE,
            MAP_SHARED, shm_fd, 0);
    if (houses == MAP_FAILED)
        ERR("mmap");

    for(int i = 0; i < n; i++){
        printf("[%d: %d] ", i, houses[i]);
    }
    munmap(houses, size);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    
    pthread_mutex_destroy(&kwatery->fh_mutex);
    pthread_cond_destroy(&kwatery->fh_cond);
    munmap(kwatery, sizeof(kwatery_t));
    exit(EXIT_SUCCESS);
}