#include "common.h"
#include "board_utils.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define BOARD_FILE "board"
#define FIFO_NAME "fifo"
#define STEP_COUNT 20
#define WAIT_N 10

#define PORT 12345

#define EPOLL_MAX_EVENTS 10

void usage(char* program_name)
{
    fprintf(stderr, "Usage: \n");

    fprintf(stderr, "\t%s n m\n", program_name);
    fprintf(stderr, "\t  n, m - board width and height, respectively\n");

    exit(EXIT_FAILURE);
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

int main(int argc, char** argv) {
    char* name = argv[0];

    if(argc != 3){
        usage(name); 
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    if(n < 2 || m < 2){
        usage(name); 
    }

    pthread_mutex_t *mutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mutex == MAP_FAILED) {
        ERR("mmap mutex");
    }
    pthread_mutexattr_t mutex_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);

    pthread_mutex_init(mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    char* board;
    ssize_t size = m*(n+1);

    int fd = open(BOARD_FILE, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if(fd < 0){
        ERR("open");
    }

    if(ftruncate(fd, size) < 0){
        ERR("ftruncate");
    }

    board = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(board == MAP_FAILED){
        ERR("mmap");
    }

    fill_board(board, n, m);

    int x, y;

    if (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0)
        if (errno != EEXIST)
            ERR("create fifo");

    int f = fork();
    switch(f){
        case 0:{
            srand(getpid());
            x = rand()%n;
            y = rand()%m;

            int fifo_read = open(FIFO_NAME, O_RDONLY);
            if(fifo_read < 0){
                ERR("open fifo");
            }

            ms_sleep(WAIT_N*100);
            char move;
            while(read(fifo_read, &move, sizeof(char)) > 0){
                printf("%c nie ze mna te numery\n", move);
                ms_sleep(100);
                safe_lock(mutex);
                if(has_trail(board, x, y, n, m)){
                    set_char(board, x, y, n, m, ' ');
                }
                else{
                    printf("Stracilem ich karamba\n");
                    set_char(board, x, y, n, m, '.');
                }
                move = get_trail_move(board, x, y, n, m);
                move_pos(board, move, n, m, &x, &y);
                pthread_mutex_unlock(mutex);
                int r;
                int br = 0;  
               
            }

            munmap(board, size);
            close(fd);
            close(fifo_read);
            exit(EXIT_SUCCESS);
        }
        case -1:
        {
            ERR("fork");
        }
    }

    srand(getpid());
    x = rand()%n;
    y = rand()%m;

    int fifo = open(FIFO_NAME, O_WRONLY);
    if(fifo < 0){
        ERR("open fifo");
    }
    for(int i = 0; i < STEP_COUNT; i++){
        safe_lock(mutex);
        set_char(board, x, y, n, m, '=');
        char move = get_random_move(board, x, y, n, m);
        move_pos(board, move, n, m, &x, &y);
        set_char(board, x, y, n, m, 'S');
        pthread_mutex_unlock(mutex);

        move = get_random_move(board, x, y, n, m);
        write(fifo, &move, sizeof(char));
        ms_sleep(100);
    }

    printf("Ekspedycja zakonczona\n");

    munmap(board, size);
    close(fd);
    pthread_mutex_destroy(mutex);
    close(fifo);
    unlink(FIFO_NAME);

    exit(EXIT_SUCCESS);
}