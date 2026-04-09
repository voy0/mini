#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERR(source)                                                            \
  (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source),             \
   kill(0, SIGKILL), exit(EXIT_FAILURE))

#define SHM_SIZE 1024

void usage(char *name) {
  fprintf(stderr, "USAGE: %s N path_to_file\n", name);
  fprintf(stderr, "1 <= N <= 10\n");
  exit(EXIT_FAILURE);
}

typedef struct {
  pthread_mutex_t mutex;
  int chars[256];
} s_count;

int main(int argc, char **argv) {
  if (argc != 3) {
    usage(argv[0]);
  }

  int n = atoi(argv[1]);
  if (n < 1 || n > 10)
    usage(argv[0]);

  struct stat st;
  if (stat(argv[2], &st) < 0)
    ERR("fstat");

  off_t file_size = st.st_size;
  if (file_size == 0) {
    return EXIT_SUCCESS;
  }

  int shm_fd = shm_open("/count_data", O_CREAT | O_EXCL | O_RDWR, 0666);
  if (shm_fd < 0)
    ERR("shm_open");

  if (ftruncate(shm_fd, sizeof(s_count)) == -1)
    ERR("ftruncate");

  s_count *count_shm_ptr;
  count_shm_ptr = (s_count *)mmap(NULL, sizeof(s_count), PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm_fd, 0);
  if (count_shm_ptr == MAP_FAILED)
    ERR("mmap");

  for (int i = 0; i < 256; i++) {
    count_shm_ptr->chars[i] = 0;
  }

  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
  pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);
  pthread_mutex_init(&count_shm_ptr->mutex, &mutex_attr);

  // char* shm_ptr;
  // if((shm_ptr = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0))
  // == MAP_FAILED) ERR("mmap");

  // if(write(STDOUT_FILENO, shm_ptr, file_size) < 0)
  //     ERR("write");

  off_t chunk_size = file_size / n;
  for (int i = 0; i < n; i++) {
    int f = fork();
    switch (f) {
    case 0:
      int child_fd = open(argv[2], O_RDONLY);
      if (child_fd < 0)
        ERR("open");

      off_t start_offset = i * chunk_size;
      off_t end_offset = (i == n - 1) ? file_size : (i + 1) * chunk_size;
      off_t my_size = end_offset - start_offset;

      char *file_ptr =
          mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, child_fd, 0);
      if (file_ptr == MAP_FAILED)
        ERR("mmap");

      int local_count[256] = {0};
      for (int j = start_offset; j < end_offset; j++) {
        local_count[(unsigned char)file_ptr[j]]++;
      }

      int error = pthread_mutex_lock(&count_shm_ptr->mutex);
      if (error == EOWNERDEAD) {
        pthread_mutex_consistent(&count_shm_ptr->mutex);
      } else if (error != 0) {
        ERR("pthread_mutex_lock");
      }
      srand(getpid());

      if ((rand() & 100) < 3) {
        printf("Proces %d nagle umiera!\n", getpid());
        abort();
      }
      for (int j = 0; j < 256; j++) {
        count_shm_ptr->chars[j] += local_count[j];
      }

      pthread_mutex_unlock(&count_shm_ptr->mutex);
      munmap(file_ptr, file_size);
      close(child_fd);
      exit(EXIT_SUCCESS);
    case -1:
      ERR("fork");
    }
  }

  int success = 1;
  for (int i = 0; i < n; i++) {
    int status;
    wait(&status); // Zapisujemy raport o zgonie do zmiennej status

    // 2. KONTROLA STATUSU
    // WIFEXITED sprawdza czy program doszedł do exit() lub return
    // WEXITSTATUS sprawdza czy było to exit(EXIT_SUCCESS)
    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
      success = 0; // Jeśli cokolwiek poszło nie tak, oblewamy całe obliczenia!
    }
  }

  // 3. DECYZJA O WYPISANIU
  if (success == 1) {
    for (int i = 0; i < 256; i++) {
      if (count_shm_ptr->chars[i] != 0) {
        if (isprint(i)) {
          printf("Znak '%c': %d razy\n", (char)i, count_shm_ptr->chars[i]);
        } else {
          printf("Znak [HEX %02X]: %d razy\n", i, count_shm_ptr->chars[i]);
        }
      }
    }
  } else {
    // Dziecko umarło, więc wypisujemy komunikat błędu
    printf("Obliczenia się nie powiodły.\n");
  }

  pthread_mutexattr_destroy(&mutex_attr);
  pthread_mutex_destroy(&count_shm_ptr->mutex);

  // UWAGA: POPRAWIONY BŁĄD Z MUNMAP, O KTÓRYM PISAŁEM WCZEŚNIEJ!
  if (munmap(count_shm_ptr, sizeof(s_count)) < 0)
    ERR("munmap");
  if (shm_unlink("/count_data") == -1)
    ERR("shm_unlink");

  return EXIT_SUCCESS;
}