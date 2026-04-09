#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SHOP_FILENAME "./shop"
#define MIN_SHELVES 8
#define MAX_SHELVES 256
#define MIN_WORKERS 1
#define MAX_WORKERS 64

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
    fprintf(stderr, "\t  n - number of items (shelves), %d <= n <= %d\n", MIN_SHELVES, MAX_SHELVES);
    fprintf(stderr, "\t  m - number of workers, %d <= m <= %d\n", MIN_WORKERS, MAX_WORKERS);
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

void swap(int* x, int* y)
{
    int tmp = *y;
    *y = *x;
    *x = tmp;
}

void shuffle(int* array, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        swap(&array[i], &array[j]);
    }
}

void print_array(int* array, int n)
{
    for (int i = 0; i < n; ++i)
    {
        printf("%3d ", array[i]);
    }
    printf("\n");
}

typedef struct
{
    int is_sorted;
    int workers_alive;
    int workers_dead[64];
    pthread_mutex_t mutex[256];
} shop_t;
void safe_lock(pthread_mutex_t* mutex)
{
    int err = pthread_mutex_lock(mutex);
    if (err != 0)
    {
        if (err == EOWNERDEAD)
        {
            pthread_mutex_consistent(mutex);
        }
        else
        {
            ERR("pthread_mutex_lock");
        }
    }
}
int main(int argc, char** argv)
{
    char* prog_name = argv[0];
    if (argc != 3)
        usage(prog_name);
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    if (N < 8 || N > 256 || M < 1 || M > 64)
        usage(prog_name);

    char* shm_name = "/SHOP_FILENAME";
    int shm_fd;
    int shop_size = sizeof(shop_t);

    shop_t* shop;

    int* products;
    int prod_size = N * sizeof(int);
    if ((shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0666)) < 0)
        ERR("shm_open");

    if (ftruncate(shm_fd, prod_size) < 0)
        ERR("ftruncate");

    products = (int*)mmap(NULL, prod_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if (products == MAP_FAILED)
        ERR("mmap");

    shop = (shop_t*)mmap(NULL, shop_size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (shop == MAP_FAILED)
        ERR("mmap");

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);

    shop->is_sorted = 0;
    shop->workers_alive = M;
    for (int i = 0; i < M; i++)
    {
        shop->workers_dead[i] = 0;
    }
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_init(&shop->mutex[i], &mutex_attr);
        products[i] = i;
    }
    pthread_mutexattr_destroy(&mutex_attr);

    shuffle(products, N);
    print_array(products, N);

    for (int i = 0; i < M; i++)
    {
        int f = fork();
        switch (f)
        {
            case 0:
                printf("%d: Reporting for the night shift...\n", getpid());

                srand(getpid());
                while (1)
                {
                    int n1 = rand() % N;
                    int n2;
                    do
                        n2 = rand() % N;
                    while (n1 == n2);
                    if (n1 > n2)
                        swap(&n1, &n2);

                    int err = pthread_mutex_lock(&shop->mutex[n1]);
                    if (err != 0)
                    {
                        if (err == EOWNERDEAD)
                        {
                            printf("%d: Found a dead body in aisle %d\n", getpid(), n1);
                            pthread_mutex_consistent(&shop->mutex[n1]);
                            shop->workers_dead[i]++;
                        }
                        else
                        {
                            ERR("pthread_mutex_lock");
                        }
                    }
                    err = pthread_mutex_lock(&shop->mutex[n2]);
                    if (err != 0)
                    {
                        if (err == EOWNERDEAD)
                        {
                            printf("%d: Found a dead body in aisle %d\n", getpid(), n2);
                            pthread_mutex_consistent(&shop->mutex[n2]);
                            shop->workers_dead[i]++;
                        }
                        else
                        {
                            ERR("pthread_mutex_lock");
                        }
                    }

                    if (rand() % 200 < 1)
                    {
                        printf("%d: Trips over a palet and dies\n", getpid());
                        abort();
                    }

                    if (products[n1] > products[n2])
                    {
                        usleep(10000);
                        swap(&products[n1], &products[n2]);
                    }
                    int br = 0;
                    if (shop->is_sorted == 1)
                    {
                        br = 1;
                    }
                    pthread_mutex_unlock(&shop->mutex[n1]);
                    pthread_mutex_unlock(&shop->mutex[n2]);
                    if (br == 1)
                        break;
                }

                exit(EXIT_SUCCESS);
            case -1:
                ERR("fork");
        }
    }
    int f = fork();
    switch (f)
    {
        case 0:
            printf("%d: Manager reports for the night shift\n", getpid());
            while (1)
            {
                usleep(5000);

                for (int i = 0; i < N; i++)
                {
                    safe_lock(&shop->mutex[i]);
                }
                print_array(products, N);
                msync(products, prod_size, MS_SYNC);

                for (int i = 0; i < M; i++)
                {
                    if (shop->workers_dead[i] > 0)
                    {
                        shop->workers_dead[i] = 0;
                        shop->workers_alive--;
                    }
                }
                printf("%d: Workers alive: %d\n", getpid(), shop->workers_alive);
                if (shop->workers_alive <= 0)
                {
                    printf("%d: All of the workers died, I hate my job\n", getpid());
                    break;
                }
                int sort = 0;
                for (int i = 0; i < N - 1; i++)
                {
                    if (products[i] > products[i + 1])
                    {
                        sort = 1;
                    }
                }
                int br = 0;
                if (sort == 0)
                {
                    printf("%d: We're all done guys, time to go home\n", getpid());
                    shop->is_sorted = 1;

                    br = 1;
                }
                for (int i = 0; i < N; i++)
                {
                    pthread_mutex_unlock(&shop->mutex[i]);
                }
                if (br == 1)
                    break;
            }
            exit(EXIT_SUCCESS);
        case -1:
            ERR("fork");
    }

    for (int i = 0; i < M + 1; i++)
    {
        wait(NULL);
    }
    print_array(products, N);
    printf("Night shift in Bitronka is over\n");

    for (int i = 0; i < N; i++)
    {
        pthread_mutex_destroy(&shop->mutex[i]);
    }

    shm_unlink(shm_name);
    munmap(shop, shop_size);
    munmap(products, prod_size);
}
