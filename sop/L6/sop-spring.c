#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                            \
  do {                                                                         \
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);                            \
    perror(source);                                                            \
    kill(0, SIGKILL);                                                          \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define KEYBOARD_CAP 6

void usage(char *name) {
  fprintf(stderr, "USAGE: %s n m k\n", name);
  fprintf(stderr, "%d <= n <= 20 - student count\n", KEYBOARD_CAP);
  fprintf(stderr, "1 <= m <= 5 - keyboard count\n");
  fprintf(stderr, "5 <= k <= %d - key count\n", KEYBOARD_CAP);

  exit(EXIT_FAILURE);
}

typedef struct {
  pthread_barrier_t barrier;
  int run;
  pthread_mutex_t run_mutex;
  pthread_mutex_t mutex[5][KEYBOARD_CAP];
} keyboard_t;
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
  char *name = argv[0];
  if (argc != 4)
    usage(name);
  int n, m, k;
  int nn = 12;

  n = atoi(argv[1]);
  m = atoi(argv[2]);
  k = atoi(argv[3]);

  if (n < KEYBOARD_CAP || n > 20 || m < 1 || m > 5 || k < 5 || k > KEYBOARD_CAP)
    usage(name);

  for (int i = 0; i < m; i++) {
    char sem_name[nn];
    snprintf(sem_name, nn, "/sop-sem-%d", i + 1);

    if (sem_unlink(sem_name) < 0) {
      if (errno != ENOENT) {
        ERR("sem_unlink");
      }
    }
  }
  char *shm_name = "SHARED_MEM_NAME";

  keyboard_t *keyboard;
  int keyboard_size = sizeof(keyboard_t);

  keyboard = (keyboard_t *)mmap(NULL, keyboard_size, PROT_WRITE | PROT_READ,
                                MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  if (keyboard == MAP_FAILED)
    ERR("mmap");

  pthread_barrierattr_t barrier_attr;
  pthread_barrierattr_init(&barrier_attr);
  pthread_barrierattr_setpshared(&barrier_attr, PTHREAD_PROCESS_SHARED);

  pthread_barrier_init(&keyboard->barrier, &barrier_attr, n + 1);

  pthread_barrierattr_destroy(&barrier_attr);

  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
  pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < k; j++) {
      pthread_mutex_init(&keyboard->mutex[i][j], &mutex_attr);
    }
  }
  pthread_mutex_init(&keyboard->run_mutex, &mutex_attr);

  keyboard->run = 0;

  for (int i = 0; i < n; i++) {
    int f = fork();
    switch (f) {
    case 0: {
      srand(getpid());

      pthread_barrier_wait(&keyboard->barrier);

      int shm_fd;

      shm_fd = shm_open(shm_name, O_RDWR, 0666);
      if (shm_fd < 0)
        ERR("shm_open");

      int shm_size = sizeof(double) * m * k;
      if (ftruncate(shm_fd, shm_size) < 0)
        ERR("truncate");

      double (*shm_arr)[k] =
          mmap(NULL, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
      if (shm_arr == MAP_FAILED)
        ERR("mmap");

      sem_t **semaphores = malloc(sizeof(sem_t *) * m);
      if (semaphores == 0)
        ERR("malloc");
      for (int j = 0; j < m; j++) {
        char sem_name[nn];
        snprintf(sem_name, nn, "/sop-sem-%d", j + 1);
        semaphores[j] = sem_open(sem_name, O_CREAT, 0666, KEYBOARD_CAP);
        if (semaphores[j] == SEM_FAILED)
          ERR("sem_open");
      }
      while (1) {
        safe_lock(&keyboard->run_mutex);
        if (keyboard->run == 1) {
          pthread_mutex_unlock(&keyboard->run_mutex);
          break;
        }
        pthread_mutex_unlock(&keyboard->run_mutex);
        int keyboard_num = rand() % m;
        int key_num = rand() % k;
        sem_wait(semaphores[keyboard_num]);

        int err = pthread_mutex_lock(&keyboard->mutex[keyboard_num][key_num]);
        if (err != 0) {
          if (err == EOWNERDEAD) {
            printf("Student %d: ktos tu lezy, ratunku !!!1!1\n", getpid());
            pthread_mutex_consistent(&keyboard->mutex[keyboard_num][key_num]);

            safe_lock(&keyboard->run_mutex);
            keyboard->run = 1;
            pthread_mutex_unlock(&keyboard->run_mutex);
          } else {
            ERR("pthread_mutex_lock");
          }
        }
        printf("Student %d: sprzątam klawiature %d klawisz %d\n", getpid(),
               keyboard_num + 1, key_num);
        usleep(300 * 1000);

        if (rand() % 100 == 1) {
          printf("Student %d: nie ma juz siły\n", getpid());
          sem_post(semaphores[keyboard_num]);
          abort();
        }
        shm_arr[keyboard_num][key_num] /= 3;
        pthread_mutex_unlock(&keyboard->mutex[keyboard_num][key_num]);
        sem_post(semaphores[keyboard_num]);
      }
      for (int j = 0; j < m; j++) {
        sem_close(semaphores[j]);
      }
      free(semaphores);
      munmap(shm_arr, shm_size);
      exit(EXIT_SUCCESS);
    }
    case -1:
      ERR("fork");
    }
  }

  int shm_fd;

  shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
  if (shm_fd < 0)
    ERR("shm_open");

  int shm_size = sizeof(double) * m * k;
  if (ftruncate(shm_fd, shm_size) < 0)
    ERR("truncate");

  double (*shm_arr)[k] =
      mmap(NULL, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
  if (shm_arr == MAP_FAILED)
    ERR("mmap");

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < k; j++) {
      shm_arr[i][j] = 1.0;
    }
  }
  usleep(500 * 1000);
  pthread_barrier_wait(&keyboard->barrier);
  for (int i = 0; i < n; i++) {
    wait(NULL);
  }
  for (int i = 0; i < m; i++) {
    char sem_name[nn];
    snprintf(sem_name, nn, "/sop-sem-%d", i + 1);
    sem_unlink(sem_name);
  }
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < k; j++) {
      printf("Klawiatura %d, klawisz %d jest wyspszątana w %f\n", i + 1, j + 1,
             100 - shm_arr[i][j]);
    }
  }
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < k; j++) {
      pthread_mutex_destroy(&keyboard->mutex[i][j]);
    }
  }
  pthread_mutex_destroy(&keyboard->run_mutex);
  pthread_barrier_destroy(&keyboard->barrier);
  munmap(keyboard, keyboard_size);
  printf("Sprzątanie zakończone!\n");
}