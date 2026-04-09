#define _GNU_SOURCE

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR(source)                                                            \
  (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),             \
   kill(0, SIGKILL), exit(EXIT_FAILURE))

// Values of this function are in range (0,1]
double func(double x) {
  usleep(2000);
  return exp(-x * x);
}

/**
 * It counts hit points by Monte Carlo method.
 * Use it to process one batch of computation.
 * @param N Number of points to randomize
 * @param a Lower bound of integration
 * @param b Upper bound of integration
 * @return Number of points which was hit.
 */
int randomize_points(int N, float a, float b) {
  int result = 0;
  for (int i = 0; i < N; ++i) {
    double rand_x = ((double)rand() / RAND_MAX) * (b - a) + a;
    double rand_y = ((double)rand() / RAND_MAX);
    double real_y = func(rand_x);

    if (rand_y <= real_y)
      result++;
  }
  return result;
}

/**
 * This function calculates approximation of integral from counters of hit and
 * total points.
 * @param total_randomized_points Number of total randomized points.
 * @param hit_points Number of hit points.
 * @param a Lower bound of integration
 * @param b Upper bound of integration
 * @return The approximation of integral
 */
double summarize_calculations(uint64_t total_randomized_points,
                              uint64_t hit_points, float a, float b) {
  return (b - a) * ((double)hit_points / (double)total_randomized_points);
}

/**
 * This function locks mutex and can sometime die (it has 2% chance to die).
 * It cannot die if lock would return an error.
 * It doesn't handle any errors. It's users responsibility.
 * Use it only in STAGE 4.
 *
 * @param mtx Mutex to lock
 * @return Value returned from pthread_mutex_lock.
 */
int random_death_lock(pthread_mutex_t *mtx) {
  int ret = pthread_mutex_lock(mtx);
  if (ret)
    return ret;

  // 2% chance to die
  if (rand() % 50 == 0) {
    fprintf(stderr, "%d: Tired of this shit gl @everyone\n", getpid());
    abort();
  }
  return ret;
}

void usage(char *argv[]) {
  printf("%s a b N - calculating integral with multiple processes\n", argv[0]);
  printf("a - Start of segment for integral (default: -1)\n");
  printf("b - End of segment for integral (default: 1)\n");
  printf("N - Size of batch to calculate before reporting to shared memory "
         "(default: 1000)\n");
  exit(EXIT_SUCCESS);
}

volatile sig_atomic_t the_end = 0;

int sethandler(void (*f)(int), int sigNo) {
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = f;
  if (-1 == sigaction(sigNo, &act, NULL))
    return -1;
  return 0;
}

void sigint_handler(int sig) { the_end = 1; }

typedef struct {
  pthread_mutex_t mutex;

  int processes;
  int samples_total;
  int samples_hit;
  int a;
  int b;
} proc_t;

void risky_lock(proc_t *proc) {
  int err = random_death_lock(&proc->mutex);
  if (err != 0) {
    if (err == EOWNERDEAD) {
      proc->processes--;
      printf("%d: Bro someone left, i gotta fix this\n", getpid());
      pthread_mutex_consistent(&proc->mutex);
    } else {
      ERR("pthread_mutex_lock");
    }
  }
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

int main(int argc, char *argv[]) {
  srand(getpid());

  sethandler(sigint_handler, SIGINT);
  int a, b, N;
  a = -1;
  b = 1;
  N = 1000;
  if (argc > 4)
    usage(argv);

  if (argc >= 2) {
    a = atoi(argv[1]);
  }
  if (argc >= 3) {
    b = atoi(argv[2]);
  }
  if (argc == 4) {
    N = atoi(argv[3]);
  }

  if (a > b)
    usage(argv);
  if (N < 1)
    usage(argv);

  pthread_mutexattr_t mutex_attr;
  proc_t *proc;
  char *sem_name = "mmap_semaphore";
  char *shm_name = "/obliczenia";

  sem_t *semaphore = sem_open(sem_name, O_CREAT, 0666, 1);
  if (semaphore == SEM_FAILED)
    ERR("sem_open");

  sem_wait(semaphore);

  int shm_fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0666);

  if (shm_fd >= 0) {

    if (ftruncate(shm_fd, sizeof(proc_t)) == -1)
      ERR("ftruncate");

    proc = (proc_t *)mmap(NULL, sizeof(proc_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_fd, 0);
    if (proc == MAP_FAILED)
      ERR("mmap");

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);

    pthread_mutex_init(&proc->mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    proc->processes = 0;
    proc->a = a;
    proc->b = b;
  } else if (errno == EEXIST) {
    shm_fd = shm_open("/obliczenia", O_RDWR, 0666);

    proc = (proc_t *)mmap(NULL, sizeof(proc_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_fd, 0);

    if (proc == MAP_FAILED)
      ERR("mmap");

    if (proc->a != a || proc->b != b) {
      fprintf(stderr, "Blad: zle granice");
      munmap(proc, sizeof(proc_t));
      sem_post(semaphore);
      abort();
    }
  } else {
    ERR("shm_open");
  }

  sem_post(semaphore);

  risky_lock(proc);
  proc->processes++;
  int current_processes = proc->processes;
  pthread_mutex_unlock(&proc->mutex);

  printf("%d: %d processes working\n", getpid(), current_processes);
  sleep(2);

  for (int i = 0; i < 3; i++) {
    if (the_end == 1) {
      break;
    }

    int hits = 0;
    int samples = 0;
    hits = randomize_points(N, a, b);

    risky_lock(proc);
    proc->samples_hit += hits;
    proc->samples_total += N;
    hits = proc->samples_hit;
    samples = proc->samples_total;
    pthread_mutex_unlock(&proc->mutex);

    float sum = summarize_calculations(samples, hits, a, b);
    printf("%d: Samples hit: %d\t; Samples Total: %d\t; Curr. Approx.: %f\t\n",
           getpid(), hits, samples, sum);
  }

  risky_lock(proc);
  int processes_left = --proc->processes;
  pthread_mutex_unlock(&proc->mutex);

  if (processes_left == 0) {

    pthread_mutex_destroy(&proc->mutex);

    shm_unlink(shm_name);
    sem_unlink(sem_name);
    printf("%d: Bylem ostatni, posprzatalem!\n", getpid());
  }
  munmap(proc, sizeof(proc_t));

  return EXIT_SUCCESS;
}