#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096
#define DEFAULT_ARRAYSIZE 10
#define DELETED_ITEM -1
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

// args array shared between threads
typedef struct argsSignalHandler
{
    pthread_t tid;                  // id of a thread
    int *pArrayCount;               
    int *array;
    pthread_mutex_t *pmxArray;      
    sigset_t *pMask;                // sigset of a thread
    bool *pQuitFlag;
    pthread_mutex_t *pmxQuitFlag;
} argsSignalHandler_t;

void ReadArguments(int argc, char **argv, int *arraySize);
void removeItem(int *array, int *arrayCount, int index);
void printArray(int *array, int arraySize);
void *signal_handling(void *);

int main(int argc, char **argv)
{
    int arraySize, *array;
    bool quitFlag = false;

    pthread_mutex_t mxQuitFlag = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxArray = PTHREAD_MUTEX_INITIALIZER;
    
    // set size of array, default_arraysize = 10
    ReadArguments(argc, argv, &arraySize);
    
    int arrayCount = arraySize; // 10
    
    // alocate memory for dynamic array
    if (NULL == (array = (int *)malloc(sizeof(int) * arraySize)))
        ERR("Malloc error for array!");

    // set every element of array as index
    for (int i = 0; i < arraySize; i++)
        array[i] = i + 1;

    // footjob with signals
    sigset_t oldMask, newMask;

    sigemptyset(&newMask);          // initialize and empty sigset
    sigaddset(&newMask, SIGINT);    // add SIGINT SIGQUIT to sigset
    sigaddset(&newMask, SIGQUIT);

    // add SIGINT SIGQUIT blocking to old sigset
    if (pthread_sigmask(SIG_BLOCK, &newMask, &oldMask))
        ERR("SIG_BLOCK error");


    // initialize argsSignalHandler_t arguments struct
    argsSignalHandler_t args;
    args.pArrayCount = &arrayCount; // 10
    args.array = array; // fresly malloced array with values [1, 2, 3, ..., 10]
    args.pmxArray = &mxArray; // mutex initialized on this 
    args.pMask = &newMask; // sigmask
    args.pQuitFlag = &quitFlag; // quit flag: false
    args.pmxQuitFlag = &mxQuitFlag; // mutex initialized on this too 

    // create single thread doing signal_handling function
    if (pthread_create(&args.tid, NULL, signal_handling, &args))
        ERR("Couldn't create signal handling thread!");
    
    // check if SIGQUIT was received by the thread and it changed
    // mxQuitFlag to true
    // if else keep printing array
    while (true)
    {
        pthread_mutex_lock(&mxQuitFlag);
        if (quitFlag == true)
        {
            pthread_mutex_unlock(&mxQuitFlag);
            break;
        }
        else
        {
            pthread_mutex_unlock(&mxQuitFlag);
            pthread_mutex_lock(&mxArray);
            printArray(array, arraySize);
            pthread_mutex_unlock(&mxArray);
            sleep(1);
        }
    }
    if (pthread_join(args.tid, NULL))
        ERR("Can't join with 'signal handling' thread");
    free(array);
    if (pthread_sigmask(SIG_UNBLOCK, &newMask, &oldMask))
        ERR("SIG_BLOCK error");
    exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char **argv, int *arraySize)
{
    *arraySize = DEFAULT_ARRAYSIZE;

    if (argc >= 2)
    {
        *arraySize = atoi(argv[1]);
        if (*arraySize <= 0)
        {
            printf("Invalid value for 'array size'");
            exit(EXIT_FAILURE);
        }
    }
}

void removeItem(int *array, int *arrayCount, int index)
{
    // *array is our array
    // *arrayCount is number of elements in array
    // index is index of element to be removed
    int curIndex = -1;
    int i = -1;

    // while current index is not index to be deleted
    while (curIndex != index)
    {
        i++;
        if (array[i] != DELETED_ITEM) // if current element is not deleted
            curIndex++;
    }
    array[i] = DELETED_ITEM;
    *arrayCount -= 1;
}

void printArray(int *array, int arraySize)
{
    printf("[");
    for (int i = 0; i < arraySize; i++)
        if (array[i] != DELETED_ITEM)
            printf(" %d", array[i]);
    printf(" ]\n");
}

void *signal_handling(void *voidArgs)
{
    // single thread that is dedicated to read signals
    
    // *args are assigned to arguments struct passed during 
    // pthread_create 
    argsSignalHandler_t *args = voidArgs;
    int signo;
    srand(time(NULL));
    for (;;)
    {
        // wait for a signal and assign signo to singal number
        // sigmask blocks the signal from terminating the thread
        if (sigwait(args->pMask, &signo))
            ERR("sigwait failed.");

        // depending on number of signal
        switch (signo)
        {
            case SIGINT:
                // remove random argument from array

                // lock mutex first that contains elements of array
                pthread_mutex_lock(args->pmxArray);
                if (*args->pArrayCount > 0)
                    removeItem(args->array, args->pArrayCount, rand() % (*args->pArrayCount));
                pthread_mutex_unlock(args->pmxArray);
                break;
            case SIGQUIT:
                // set quit flag to true if SIGQUIT signal was received
                pthread_mutex_lock(args->pmxQuitFlag);
                *args->pQuitFlag = true;
                pthread_mutex_unlock(args->pmxQuitFlag);
                return NULL;
            default:
                printf("unexpected signal %d\n", signo);
                exit(1);
        }
    }
    return NULL;
}