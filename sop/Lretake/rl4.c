#include "common.h"
#include "board_utils.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>

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

            // --- ETAP 4: Konfiguracja gniazda TCP ---
            int server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd < 0) ERR("socket");

            int opt = 1;
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORT);
            addr.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) ERR("bind");
            if (listen(server_fd, 10) < 0) ERR("listen");

            // --- ETAP 4: Konfiguracja epoll ---
            int epoll_fd = epoll_create1(0);
            if (epoll_fd < 0) ERR("epoll_create1");

            struct epoll_event ev;
            
            // Rejestrujemy plik FIFO
            ev.events = EPOLLIN;
            ev.data.fd = fifo_read;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fifo_read, &ev) < 0) ERR("epoll_ctl fifo");

            // Rejestrujemy gniazdo TCP
            ev.events = EPOLLIN;
            ev.data.fd = server_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) ERR("epoll_ctl server");

            struct epoll_event events[EPOLL_MAX_EVENTS];

            ms_sleep(WAIT_N*100);
            
            int running = 1;
            while(running) {
                // Czekamy asynchronicznie na którekolwiek ze zdarzeń
                int nfds = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);
                if (nfds < 0) {
                    if (errno == EINTR) continue;
                    ERR("epoll_wait");
                }

                for(int i = 0; i < nfds; i++) {
                    // Zdarzenie na FIFO - ruch ekspedycji
                    if (events[i].data.fd == fifo_read) {
                        char move;
                        int bytes_read = read(fifo_read, &move, sizeof(char));
                        
                        if (bytes_read > 0) {
                            // Dokładne wypisanie stringów z treści zadań 2 i 3
                            printf("Kierunek %c? Nie ze mna te numery, karramba!\n", move);
                            
                            safe_lock(mutex);
                            if(has_trail(board, x, y, n, m)){
                                set_char(board, x, y, n, m, ' ');
                            }
                            else{
                                printf("Karramba!\n");
                                set_char(board, x, y, n, m, '.');
                            }
                            char trail_move = get_trail_move(board, x, y, n, m);
                            move_pos(board, trail_move, n, m, &x, &y);
                            pthread_mutex_unlock(mutex);
                        } 
                        else if (bytes_read == 0) {
                            // Rodzic zamknął FIFO (koniec podróży)
                            running = 0;
                        } 
                        else {
                            ERR("read");
                        }
                    } 
                    // Zdarzenie na gnieździe sieciowym TCP - wiadomość z Krainy Deszczowców
                    else if (events[i].data.fd == server_fd) {
                        int client_fd = accept(server_fd, NULL, NULL);
                        if (client_fd < 0) ERR("accept");
                        
                        printf("Polaczono z dowodztwem -- odbiór!\n");
                        
                        close(client_fd); // Zamknięcie zgodnie z poleceniem
                    }
                }
            }

            // Sprzątanie po wyjściu z nieskończonej pętli
            close(epoll_fd);
            close(server_fd);
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

    // --- PROCES MACIERZYSTY (Smok Wawelski) ---
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

    printf("Expedycja zakończona!\n"); // String zgodnie z Etapem 1

    munmap(board, size);
    close(fd);
    pthread_mutex_destroy(mutex);
    munmap(mutex, sizeof(pthread_mutex_t));
    
    // Zamykając FIFO u rodzica, read() w dziecku zwróci 0 i dziecko ucieknie z pętli
    close(fifo); 
    wait(NULL); // Czekamy aż dziecko (Don Pedro) posprząta i się wyłączy
    unlink(FIFO_NAME); // Ostatni punkt z polecenia - usunięcie pliku kolejki

    exit(EXIT_SUCCESS);
}