#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096
#define DEFAULT_N 1000
#define DEFAULT_K 10
#define BIN_COUNT 11
#define NEXT_DOUBLE(seedptr) ((double)rand_r(seedptr) / (double)RAND_MAX)
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef unsigned int UINT;

// struct that will be shared with all threads
typedef struct argsThrower
{
    pthread_t tid;
    UINT seed;
    int* pBallsThrown;
    int* pBallsWaiting;
    int* bins;
    // objects to be locked by mutex
    pthread_mutex_t* pmxBallsThrown;
    pthread_mutex_t* pmxBallsWaiting;
    pthread_mutex_t* mxBins;

    int* ballsThrownBySingleMutex;
}argsThrower_t;

void ReadArguments(int argc, char **argv, int *ballsCount, int *throwersCount);
void make_throwers(argsThrower_t *argsArray, int throwersCount);
void *throwing_func(void *args);
int throwBall(UINT *seedptr);

int main(int argc, char** argv)
{
    // reading arguments
    int ballsCount, throwersCount;
    ReadArguments(argc, argv, &ballsCount, &throwersCount);

    // initializing for balls to be thrown
    int ballsThrown = 0, bt = 0;
    int ballsWaiting = ballsCount;

    // initializing mutexes
    pthread_mutex_t main_mutex_BallsThrown = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t main_mutex_BallsWaiting = PTHREAD_MUTEX_INITIALIZER;

    int bins[BIN_COUNT];
    pthread_mutex_t mxBins[BIN_COUNT];

    // initalizing mutexes on each bin in bins
    // without using PTHREAD_MUTEX_INITIALIZE macro
    // maybe because it doesnt set the memory right idk
    // setting 0 to all indexes of bins
    for(int i = 0 ; i < BIN_COUNT; i++)
    {
        bins[i] = 0;
        if(pthread_mutex_init(&mxBins[i], NULL))
            ERR("Couldnt initalize mutex");
    }

    // mallocowanie structury
    argsThrower_t *args = (argsThrower_t*)malloc(sizeof(argsThrower_t) * throwersCount);
    if(args == NULL)
        ERR("Malloc error for throwers arguments");
    
    srand(time(NULL));
    for(int i = 0; i < throwersCount; i++)
    {
        args[i].seed = (UINT)rand();
        args[i].pBallsThrown = &ballsThrown;
        args[i].pBallsWaiting = &ballsWaiting;
        args[i].bins = bins;
        args[i].pmxBallsThrown = &main_mutex_BallsThrown;
        args[i].pmxBallsWaiting = &main_mutex_BallsWaiting;
        args[i].mxBins = mxBins;
        args[i].ballsThrownBySingleMutex = 0;
    }

    make_throwers(args, throwersCount);

    while(bt < ballsCount)
    {
        sleep(1);
        // lock mutex while writing to it
        pthread_mutex_lock(&main_mutex_BallsThrown);
        bt = ballsThrown;
        pthread_mutex_unlock(&main_mutex_BallsThrown);
    }

    int realBallsCount = 0;
    double meanValue = 0.0;

    for(int i = 0; i < BIN_COUNT; i++)
    {
        realBallsCount += bins[i];
        meanValue += bins[i] * i;
    }
    meanValue = meanValue / realBallsCount;

    printf("Bins count:\n");
    for (int i = 0; i < BIN_COUNT; i++)
        printf("%d\t", bins[i]);
    printf("\nTotal balls count : %d\nMean value: %f\n", realBallsCount, meanValue);
    // for (int i = 0; i < BIN_COUNT; i++) pthread_mutex_destroy(&mxBins[i]);
    // free(args);
    // The resources used by detached threads cannod be freed as we are not sure
    // if they are running yet.

    exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char **argv, int *ballsCount, int *throwersCount)
{
    *throwersCount = DEFAULT_K;
    *ballsCount = DEFAULT_N;
    if(argc >= 2)
    {
        *throwersCount = atoi(argv[1]);
        if(*throwersCount <= 0)
        {
            printf("Ivalid value for `throwers count`");
            exit(EXIT_FAILURE);
        }
    }
    if(argc >= 3)
    {
        *ballsCount = atoi(argv[2]);
        if(*ballsCount <= 0)
        {
            printf("Ivalid value for `balls count`");
            exit(EXIT_FAILURE);
        }
    }
}

void make_throwers(argsThrower_t *argsArray, int throwersCount)
{
    pthread_attr_t threadAttr;
    
    // initialize pthread attribute
    if(pthread_attr_init(&threadAttr))
        ERR("counldnt create pthread_attr_t");
    
    // set detachstate on attribute
    if(pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED))
        ERR("couldnt setdetachstate on pthread_attr_t");
    
    // create throwers
    for(int i = 0; i < throwersCount; i++)
    {
        if(pthread_create(&argsArray[i].tid, &threadAttr, throwing_func, &argsArray[i]))
            ERR("couldnt create thread");
    }

    // get rid of thread attribute
    pthread_attr_destroy(&threadAttr);
}

void* throwing_func(void* voidArgs)
{
    argsThrower_t* args = voidArgs;
    while(1)
    {
        pthread_mutex_lock(args->pmxBallsWaiting);

        if(*args->pBallsWaiting > 0)
        {
            (*args->pBallsWaiting) -= 1;
            pthread_mutex_unlock(args->pmxBallsWaiting);
        }
        else
        {
            pthread_mutex_unlock(args->pmxBallsWaiting);
            break;
        }
        // choosing a bin to put the ball into
        int binno = throwBall(&args->seed);
        pthread_mutex_lock(&args->mxBins[binno]);
        args->bins[binno] += 1;
        pthread_mutex_unlock(&args->mxBins[binno]);

        // updating balls thrown 
        pthread_mutex_lock(args->pmxBallsThrown);
        (*args->pBallsThrown) += 1;
        pthread_mutex_unlock(args->pmxBallsThrown);
    }
    args->ballsThrownBySingleMutex = args->pBallsThrown;
    printf("balls thrown: %d \n", *(args->ballsThrownBySingleMutex));
    return NULL;
}

int throwBall(UINT *seedptr)
{
    int result = 0; 
    // goes through every bin index and determines bin number
    // that a ball has fallen into, does it randomizing values
    // from 0 to 1 times BIN_COUNT, result bin is mostly in the middle
    for(int i = 0; i < BIN_COUNT - 1; i++)
        if(NEXT_DOUBLE(seedptr) > 0.5)
            result++;

    return result;
}