#include "w7-common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>

#define BACKLOG 5
#define MAX_CLIENTS 4
#define MAX_CITIES 20

// Flaga wyłączająca serwer przy Ctrl+C
volatile sig_atomic_t do_work = 1;
void sigint_handler(int sig) { do_work = 0; }

void usage(char *name) { 
    fprintf(stderr, "USAGE: %s port\n", name); 
    exit(EXIT_FAILURE);
}

// Funkcja pomocnicza - bezpiecznie zamyka pokój posłańca i zwalnia krzesło
void disconnect_client(int fd, int epoll_descriptor, int* clients, int* active_clients) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == fd) {
            clients[i] = -1;
            (*active_clients)--;
            break;
        }
    }
    epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, fd, NULL);
    if (TEMP_FAILURE_RETRY(close(fd)) < 0) ERR("close");
    printf("[S]: Poslaniec (FD: %d) opuscil biblioteke. Aktywnych: %d/%d\n", fd, *active_clients, MAX_CLIENTS);
}

void doServer(int tcp_listen_socket) {
    int epoll_descriptor;
    if ((epoll_descriptor = epoll_create1(0)) < 0) {
        ERR("epoll_create");
    }

    struct epoll_event event, events[MAX_CLIENTS + 1];
    event.events = EPOLLIN;
    event.data.fd = tcp_listen_socket;
    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, tcp_listen_socket, &event) == -1) {
        ERR("epoll_ctl: listen_sock");
    }

    // Stan Serwera
    int clients[MAX_CLIENTS];
    for(int i = 0; i < MAX_CLIENTS; i++) clients[i] = -1;
    int active_clients = 0;
    
    // Tablica miast - na początku 'g' (Grecy)
    char cities[MAX_CITIES];
    for(int i = 0; i < MAX_CITIES; i++) cities[i] = 'g'; 

    // Maska sygnałów do bezpiecznego zamykania serwera przy Ctrl+C
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    printf("[S]: Biblioteka otwarta. Nasluchuje...\n");

    while (do_work) {
        int nfds = epoll_pwait(epoll_descriptor, events, MAX_CLIENTS + 1, -1, &oldmask);
        if (nfds < 0) {
            if (errno == EINTR) continue; // Przerwanie przez SIGINT
            ERR("epoll_pwait");
        }

        for (int n = 0; n < nfds; n++) {
            
            // ========================================================
            // 1. KTOŚ PUKA DO DRZWI (NOWY POSŁANIEC)
            // ========================================================
            if (events[n].data.fd == tcp_listen_socket) {
                int new_client = add_new_client(tcp_listen_socket);
                if (new_client < 0) continue;

                // Logika BRAMKARZA z etapu 2
                if (active_clients >= MAX_CLIENTS) {
                    printf("[S]: Biblioteka PELNA. Odrzucam poslance (FD: %d)\n", new_client);
                    TEMP_FAILURE_RETRY(close(new_client));
                } else {
                    // Znajdujemy wolne miejsce dla posłańca
                    for(int i = 0; i < MAX_CLIENTS; i++) {
                        if(clients[i] == -1) {
                            clients[i] = new_client;
                            active_clients++;
                            break;
                        }
                    }
                    
                    // Montujemy "kamerę" w pokoju posłańca
                    struct epoll_event client_event;
                    client_event.events = EPOLLIN;
                    client_event.data.fd = new_client;
                    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, new_client, &client_event) == -1) {
                        ERR("epoll_ctl: add client");
                    }
                    printf("[S]: Nowy poslaniec (FD: %d) wszedl. Aktywnych: %d/%d\n", new_client, active_clients, MAX_CLIENTS);
                }
            } 
            // ========================================================
            // 2. WPROWADZONO NOWE DANE (ZMIANA WŁAŚCICIELA MIASTA)
            // ========================================================
            else {
                int current_client = events[n].data.fd;
                char buf[5]; // 4 bajty + '\0' dla bezpieczenstwa
                memset(buf, 0, sizeof(buf));

                // Czytamy MAX 4 bajty
                ssize_t size = read(current_client, buf, 4);

                if (size == 0 || (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    // Rozłączenie lub EPIPE przy odczycie
                    disconnect_client(current_client, epoll_descriptor, clients, &active_clients);
                } 
                else if (size > 0) {
                    
                    // ETAP 4: Jeśli przysłali za mało bajtów -> niepoprawny format, WYWALAMY
                    if (size != 4) {
                        printf("[S]: Zly format (dlugosc %ld). Wyrzucam poslance (FD: %d)\n", size, current_client);
                        disconnect_client(current_client, epoll_descriptor, clients, &active_clients);
                        continue;
                    }

                    // ETAP 4: Weryfikacja formatu (pXX\n lub gXX\n)
                    int is_valid = 1;
                    if (buf[0] != 'p' && buf[0] != 'g') is_valid = 0;
                    if (buf[1] < '0' || buf[1] > '9') is_valid = 0;
                    if (buf[2] < '0' || buf[2] > '9') is_valid = 0;
                    if (buf[3] != '\n') is_valid = 0;

                    int city_num = -1;
                    if (is_valid) {
                        city_num = (buf[1] - '0') * 10 + (buf[2] - '0');
                        // Numer miasta poza zakresem 1-20
                        if (city_num < 1 || city_num > MAX_CITIES) is_valid = 0; 
                    }

                    if (!is_valid) {
                        printf("[S]: Zly format danych (%.4s). Wyrzucam poslance (FD: %d)\n", buf, current_client);
                        disconnect_client(current_client, epoll_descriptor, clients, &active_clients);
                    } 
                    else {
                        // DANE SĄ POPRAWNE
                        printf("[S<-%d]: %.4s", current_client, buf);
                        int city_idx = city_num - 1; // Od 0 do 19

                        // Sprawdzamy czy właściciel faktycznie się zmienił
                        if (cities[city_idx] != buf[0]) {
                            cities[city_idx] = buf[0];
                            printf("[S]: UWAGA! Miasto %02d przejete przez: %s\n", city_num, (buf[0] == 'g') ? "Grekow" : "Persow");

                            // ROZSYŁANIE WIEŚCI DO INNYCH POSŁAŃCÓW (Broadcasting)
                            for (int i = 0; i < MAX_CLIENTS; i++) {
                                int target_fd = clients[i];
                                // Nie wysyłamy do pustych krzeseł ani do tego, co przyniósł wieści
                                if (target_fd != -1 && target_fd != current_client) {
                                    
                                    if (bulk_write(target_fd, buf, 4) < 0) {
                                        // Jeśli nie udało się wysłać, a powodem jest ucieczka klienta (EPIPE)
                                        if (errno == EPIPE) {
                                            printf("[S]: Odkryto uciekiniera podczas rozsylania.\n");
                                            disconnect_client(target_fd, epoll_descriptor, clients, &active_clients);
                                        } else {
                                            ERR("write");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ========================================================
    // 3. SPRZĄTANIE (Po otrzymaniu SIGINT)
    // ========================================================
    printf("\n[S]: Otrzymano rozkaz ewakuacji (SIGINT). Zamykanie biblioteki...\n");
    
    printf("\n=== STAN TERYTORIUM ===\n");
    for(int i = 0; i < MAX_CITIES; i++) {
        printf("Miasto %02d: %s\n", i + 1, (cities[i] == 'g') ? "Grecy" : "Persowie");
    }
    printf("=======================\n");

    // Zamykamy gniazda wszystkich pozostałych posłańców
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            TEMP_FAILURE_RETRY(close(clients[i]));
        }
    }
    
    TEMP_FAILURE_RETRY(close(epoll_descriptor));
    sigprocmask(SIG_UNBLOCK, &mask, NULL); // Przywracamy domyślne sygnały
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
    }

    // 1. Ochrona serwera przed zamachem EPIPE
    if (sethandler(SIG_IGN, SIGPIPE)) ERR("setting SIGPIPE");
    // 2. Podpięcie funkcji zamykającej przy Ctrl+C
    if (sethandler(sigint_handler, SIGINT)) ERR("Setting SIGINT");

    int tcp_listen_socket = bind_tcp_socket(atoi(argv[1]), BACKLOG);
    
    doServer(tcp_listen_socket);

    if (TEMP_FAILURE_RETRY(close(tcp_listen_socket)) < 0) ERR("close");
    fprintf(stderr, "Server ma zakonczyl dzialanie.\n");
    return EXIT_SUCCESS;
}