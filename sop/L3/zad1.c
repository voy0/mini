#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define UPPER_LIMIT 50

volatile sig_atomic_t quitFlag = false;
void sig_handler(int signo)
{
    quitFlag = true;
}
void sethandler(void (*f)(int), int sigNo) 
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    
    act.sa_handler = f;
    
    //wywolanie funkcji
    if(-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");

}



typedef unsigned int UINT;
typedef struct pthreadArgs
{
    pthread_t tid;
    UINT seed;
    int* pL;
    int* threadsDone;
    // sigset_t* pMask;
    // bool* pQuitFlag;
    pthread_mutex_t* mutex_pL;
    pthread_mutex_t* mutex_threadsDone;
}pthreadArgs_t;

void* thread_work();

int main(int argc, char** argv)
{
    int n = atoi(argv[1]);

    // sigset_t oldMask, newMask;

    // sigemptyset(&newMask);
    // sigaddset(&newMask, SIGQUIT);

    // if (pthread_sigmask(SIG_BLOCK, &newMask, &oldMask))
    //     ERR("SIG_BLOCK error");

    sethandler(sig_handler, SIGINT);

    // bool quitFlag = false;

    pthread_mutex_t main_mutex_pL = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t main_mutex_threadsDone = PTHREAD_MUTEX_INITIALIZER;

    pthreadArgs_t* sex = (pthreadArgs_t*)malloc(sizeof(pthreadArgs_t)*n);

    if(sex == NULL)
        ERR("malloc");

    srand(time(NULL));

    int L = 2;
    int tD = 0;


    for(int i = 0; i < n; i++)
    {
        sex[i].seed = (UINT)rand();
        sex[i].pL = &L;
        sex[i].threadsDone = &tD;
        // sex[i].pMask = &newMask;
        // sex[i].pQuitFlag = &quitFlag;
        sex[i].mutex_pL = &main_mutex_pL;
        sex[i].mutex_threadsDone = &main_mutex_threadsDone;
    }

    for(int i = 0; i < n; i++)
    {
        if(pthread_create(&(sex[i].tid), NULL, thread_work, &sex[i]))
            ERR("pthread_create");
    }
    
    while(tD < n && quitFlag == false)
    {
        sleep(1);
        pthread_mutex_lock(&main_mutex_pL);
        L++;
        pthread_mutex_unlock(&main_mutex_pL);      
    }

    for(int i = 0; i < n; i++)
    {
        if(pthread_join(sex[i].tid, NULL))
            ERR("pthread_join");
    }
    free(sex);
    printf("\n\tAll done right\n");
    exit(EXIT_SUCCESS);
}

void* thread_work(void* voidPtr)
{
    pthreadArgs_t* args = voidPtr;
    
    int M = rand_r(&args->seed)%(UPPER_LIMIT - 1) + 2;

    pthread_mutex_lock(args->mutex_pL);
    int L = *(args->pL);
    pthread_mutex_unlock(args->mutex_pL);

    while(quitFlag == false)
    {
        pthread_mutex_lock(args->mutex_pL);
        if(M % L == 0)
        {
            pthread_mutex_unlock(args->mutex_pL);
            break;
        }
        L = *(args->pL);
        pthread_mutex_unlock(args->mutex_pL);
    }
    if(quitFlag == false)
    {
        pthread_mutex_lock(args->mutex_threadsDone);
        (*args->threadsDone)++;
        printf("%d, dzielnik: %d, threadsDone: %d\n", M, L, (*args->threadsDone));
        pthread_mutex_unlock(args->mutex_threadsDone);
    }
    return NULL;
}