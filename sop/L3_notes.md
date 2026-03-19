#  Wątki

## `<pthread.h>`

- POSIX threads
- single `process` can contain multiple `threads`
- `threads` share same global memory, but each thread has its own stack

#### they share 
- PID, parent PID (PPID), group ID, session ID
- open file descriptors
- current directory

#### `thread` attributes
- thread ID stored in `pthread_t` data type
    - `pthread-self` obtains thread ID of self
    - thread ID is returned to the caller of `pthread_create`
    - thread ID is guaranteed to be uniqe from other working threads within the process

- signal mask `pthread_sigmask`

- `errno` variable

- some signal stack `signalstack`

- real ass time scheduling policy and priority `sched`

#### `return` 
most return 0 on succes, and `errno` if failure



### `pthreads` functions

#### pthread_create 

```c
#include <pthread.h>

int pthread_create(pthread_t *restrict thread,
           const pthread_attr_t *restrict attr,
           void *(*start_routine)(void*), void *restrict arg);

// thread stores threadID
```
`sigmask` is inherited from calling thread

`*restrict` keyword specifies that this pointer and only this pointer will access the variable, no other pointer can be set to point at this variable, very protective bf
- `pthread_t thread` stores tid
- `pthread_attr_t attr` usualy we set it to NULL
- `void* start_routine` reference to a function that thread will work on
- `void* arg` pointer to any type of arguments, can be int, float, struct, whatever

```c
return EXIT_SUCCESS // 0
return EXIT_FAILURE // error number
```

#### pthread_join

```c
    #include <pthread.h>

int pthread_join(pthread_t thread, void **value_ptr);
```

- suspends execution of current thread untill `thread` terminates
- upon successful completion `**value ptr` will point to value from `pthread_exit()` 

```c
return EXIT_SUCCESS // 0
return EXIT_FAILURE // error number
```
#### `pthread_detach`

Kinda means that we give it some money and let run free into the world of program memory, its a grown idividual now and will clean up memory after itself and terminate like a good boy without the need to use `pthread_join`. Meaning we dont wait any more for a thread to finish when we terminate `main`, it will automoatically release the resources.

```c
#include <pthread.h>

int pthread_detach(pthread_t thread);
```

### `rand` functions
`rand_r` 

- return a random integer
- is thread safe unline `rand`

### `pthread_mutex` functions

Mutex is basically a flag that acts as a gate keeper to data.

#### `pthread_mutex_destroy`

```c
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_mutex_init(pthread_mutex_t *restrict mutex,
           const pthread_mutexattr_t *restrict attr);


```

`pthread_mutex_destroy` can only destroy mutex that is unlocked

`PTHREAD_MUTEX_INITIALIZER` is macro for initializing a mutex with default values, only used for static variables
when initializing a dynamic array we need to use `pthread_mutex_init` for each index

```c
return EXIT_SUCCESS // 0
return EXIT_FAILURE // error number
```

#### `pthread_mutex_lock` `pthread_mutex_unlock`

```c
#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

```

deadlocks may occur if a calling thread relocks a mutex that the thread has already locked

###

### `pthread_attr` functions

Thread attributes object is automatically initialized with `pthread_create`

#### `pthread_attr_init` 

Initialize thread attributes object

```c
#include <pthread.h>

int pthread_attr_init(pthread_attr_t *attr);
```

#### `pthread_attr_destroy`

Destroys thread attributes object

```c
#include <pthread.h>

int pthread_attr_destroy(pthread_attr_t *attr);
```
to `pthread_attr_init`, `pthread_attr_destroy` 

```c
return EXIT_SUCCESS // 0
return EXIT_FAILURE // error number
```

#### `pthread_attr_getdetachstate` `pthread_attr_setdetachstate`

```c
#include <pthread.h>

int pthread_attr_getdetachstate(const pthread_attr_t *attr,
    int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, 
    int detachstate);
```
`detachstate` attribute sets the state of a created thread to either detached state or not.

It can be set to either:
- `PTHREAD_CREATE_DETACHED` threads created with this attribute are in detached state
- `PTHREAD_CREATE_JOINABLE`  threads created with this attribute are in joinable state (default for `detachstate`)

```c
return EXIT_SUCCESS // 0
return EXIT_FAILURE // error number
```

### `pthread_cancel`
requests cancelation of a thread with tid given by `pthread_t thread`
```c
#include <pthread.h>

int pthread_cancel(pthread_t thread);
```
```c
return EXIT_SUCCESS // 0
return EXIT_FAILURE // error number
```

### `pthread_cleanup` functions

```c
 #include <pthread.h>

void pthread_cleanup_push(void (*routine)(void *),
                            void *arg);
void pthread_cleanup_pop(int execute);
```

`pthread_cleanup_push` 
- `void *routine` is clean-up handler of a calling thread stack, whenever a sub thread is cancelled, calling thread invokes the `*routine` function that might unlock a mutex or whatever it is programmed to do
- pushes the routine to the top of the stack
- when the `*routine` is later invoked it is passed `*arg`

`pthread_cleanup_pop`
- removes the routine from the stack
- if `execute` is non-zero, `*routine` is executed 

when a thread is cancelled, or calls `pthread_exit()` all of the stacked stored `*routine`'s are poped of the stack and executed


## `<signal.h>`
### `pthread_sigmask`

- equivalent to `sigprocmask` but without restriction to a single thread process

```c
#include <signal.h>

int pthread_sigmask(int how, const sigset_t *restrict set,
           sigset_t *restrict oset);
```

`int how`

 - `SIG_BLOCK` add to the blocking set
 - `SIG_SETMASK` change blocking set
 - `SIG_UNBLOCK` unblock signals from given set

 `sigset_t set` new sigset


 `sigset_t oset` old sigset
 - when `set` is null can be used to get info about current set, `how` is then non significant


### `sigprocmask`
 ```c
  #include <signal.h>

       int sigprocmask(int how, const sigset_t *restrict set,
           sigset_t *restrict oset);
```

## `<time.h>`

### `clock_getres`
returns resolution of any clock
```c
#include <time.h>

int clock_getres(clockid_t clock_id, struct timespec *res);
```