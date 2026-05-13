#include "w7-common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

#define UNIX_SK_NAME "Laurenty"
#define MAX_CLIENTS 64
#define MAX_MSG_LEN 256
#define BACKLOG 5

// Struktura przechowująca stan pojedynczego klienta
typedef struct {
    int fd;
    int state; // 0 = czeka na imię, 1 = czeka na wybranka, 2 = sparowany (chat)
    char name[MAX_MSG_LEN];
    char beloved[MAX_MSG_LEN];
    int partner_fd;
    char buf[MAX_MSG_LEN];
    int buf_len;
} Client;

Client clients[MAX_CLIENTS];

void usage(char *name) {
    fprintf(stderr, "USAGE: %s timeout\n", name);
    exit(EXIT_FAILURE);
}

// Funkcja ustawiająca gniazdo w tryb nieblokujący
void set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) ERR("fcntl get");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) ERR("fcntl set");
}

// Funkcja sprzątająca po kliencie, który uciekł
void disconnect_client(int idx, int epoll_fd) {
    if (clients[idx].state == 0) {
        printf("Utracilem kontakt z ??\n");
    } else {
        printf("Utracilem kontakt z %s\n", clients[idx].name);
    }

    // Jeśli miał partnera, zdejmujemy mu powiązanie
    if (clients[idx].partner_fd != -1) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == clients[idx].partner_fd) {
                clients[i].partner_fd = -1;
                break;
            }
        }
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clients[idx].fd, NULL);
    if (TEMP_FAILURE_RETRY(close(clients[idx].fd)) < 0) ERR("close");
    clients[idx].fd = -1; // Zwolnienie miejsca w tablicy
}

// Logika przetwarzania pojedynczej, kompletnej wiadomości od klienta
void handle_message(int idx, char *msg) {
    // ---------------------------------------------------------
    // ETAP 4: List gończy (bezpośrednie adresowanie np. Julia:Hej)
    // ---------------------------------------------------------
    char *colon = strchr(msg, ':');
    if (colon != NULL) {
        *colon = '\0'; // Dzielimy string na dwie części
        char *target_name = msg;
        char *actual_msg = colon + 1;

        int found = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            // Szukamy klienta o podanym imieniu (musi mieć state >= 1)
            if (clients[i].fd != -1 && clients[i].state >= 1 && strcmp(clients[i].name, target_name) == 0) {
                char out[MAX_MSG_LEN + 2];
                snprintf(out, sizeof(out), "%s\n", actual_msg);
                if (bulk_write(clients[i].fd, out, strlen(out)) < 0 && errno != EPIPE) {
                    ERR("write");
                }
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Bledny adresat: %s\n", target_name);
        }
        return; // Zakończ przetwarzanie, to była wiadomość specjalna
    }

    // ---------------------------------------------------------
    // ETAP 2 & 3 & 4: Standardowy przepływ (Imię -> Wybranek -> Chat)
    // ---------------------------------------------------------
    if (clients[idx].state == 0) {
        // Klient podaje swoje imię
        strncpy(clients[idx].name, msg, MAX_MSG_LEN - 1);
        clients[idx].state = 1;
    } 
    else if (clients[idx].state == 1) {
        // Klient podaje imię wybranka
        strncpy(clients[idx].beloved, msg, MAX_MSG_LEN - 1);
        printf("%s chce pobrac sie z %s\n", clients[idx].name, clients[idx].beloved);

        // ETAP 3: Próba sparowania
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (i != idx && clients[i].fd != -1 && clients[i].state >= 1) {
                // Sprawdzamy, czy miłość jest odwzajemniona
                if (strcmp(clients[i].name, clients[idx].beloved) == 0 &&
                    strcmp(clients[i].beloved, clients[idx].name) == 0) {
                    
                    // Mamy to! POBRALI SIĘ
                    clients[idx].state = 2;
                    clients[i].state = 2;
                    clients[idx].partner_fd = clients[i].fd;
                    clients[i].partner_fd = clients[idx].fd;

                    printf("%s i %s pobrali sie!\n", clients[idx].name, clients[i].name);

                    // Wysyłamy gratulacje do obojga
                    char congrats[MAX_MSG_LEN * 2];
                    snprintf(congrats, sizeof(congrats), "Gratulacje, %s i %s!\n", clients[idx].name, clients[i].name);
                    
                    if (bulk_write(clients[idx].fd, congrats, strlen(congrats)) < 0 && errno != EPIPE) ERR("write");
                    if (bulk_write(clients[i].fd, congrats, strlen(congrats)) < 0 && errno != EPIPE) ERR("write");
                    return;
                }
            }
        }
    } 
    else if (clients[idx].state == 2) {
        // ETAP 4: Przesyłanie zwykłej wiadomości do partnera
        if (clients[idx].partner_fd != -1) {
            char out[MAX_MSG_LEN + 2];
            snprintf(out, sizeof(out), "%s\n", msg);
            if (bulk_write(clients[idx].partner_fd, out, strlen(out)) < 0 && errno != EPIPE) {
                ERR("write");
            }
        } else {
            // W przypadku braku partnera wiadomość jest odrzucana (nic nie wysyłamy)
        }
    }
}

void doServer(int listen_sock, int timeout) {
    int epoll_fd;
    if ((epoll_fd = epoll_create1(0)) < 0) ERR("epoll_create");

    struct epoll_event event, events[MAX_CLIENTS + 1];
    event.events = EPOLLIN;
    event.data.fd = listen_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event) == -1) ERR("epoll_ctl: listen_sock");

    // Inicjalizacja tablicy klientów
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
    }

    while (1) {
        // Czekamy na zdarzenia z określonym timeoutem
        int nfds = epoll_wait(epoll_fd, events, MAX_CLIENTS + 1, timeout * 1000);

        if (nfds == 0) { // Zjawisko Timeoutu
            printf("Nikt juz nie potrzebuje mojej pomocy!\n");
            break;
        }
        if (nfds < 0) {
            if (errno == EINTR) continue;
            ERR("epoll_wait");
        }

        for (int n = 0; n < nfds; n++) {
            
            // 1. KTOŚ PUKA DO DRZWI (Gniazdo UNIX nasłuchujące)
            if (events[n].data.fd == listen_sock) {
                int nfd = add_new_client(listen_sock);
                if (nfd >= 0) {
                    set_nonblock(nfd);
                    printf("Kolejna mloda osoba (%d) potrzebuje mojej pomocy!\n", nfd);

                    // Szukamy wolnego "krzesła" dla klienta
                    int slot = -1;
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (clients[i].fd == -1) { slot = i; break; }
                    }

                    if (slot != -1) {
                        clients[slot].fd = nfd;
                        clients[slot].state = 0;
                        clients[slot].partner_fd = -1;
                        clients[slot].buf_len = 0;
                        memset(clients[slot].name, 0, MAX_MSG_LEN);
                        memset(clients[slot].beloved, 0, MAX_MSG_LEN);

                        struct epoll_event c_ev;
                        c_ev.events = EPOLLIN;
                        c_ev.data.fd = nfd;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, nfd, &c_ev) == -1) ERR("epoll_ctl: client");
                    } else {
                        // Jeśli brak miejsca w pamięci, zamykamy brutlanie
                        TEMP_FAILURE_RETRY(close(nfd)); 
                    }
                }
            } 
            // 2. KLIENT COŚ WYSŁAŁ LUB SIĘ ROZŁĄCZYŁ
            else {
                int c_fd = events[n].data.fd;
                int idx = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == c_fd) { idx = i; break; }
                }

                if (idx != -1) {
                    int space_left = MAX_MSG_LEN - 1 - clients[idx].buf_len;
                    // Czytamy na raty w trybie non-blocking
                    ssize_t size = read(c_fd, clients[idx].buf + clients[idx].buf_len, space_left);

                    if (size > 0) {
                        clients[idx].buf_len += size;
                        clients[idx].buf[clients[idx].buf_len] = '\0';

                        // Parsowanie bufora w poszukiwaniu nowych linii '\n'
                        char *newline;
                        while ((newline = strchr(clients[idx].buf, '\n')) != NULL) {
                            *newline = '\0'; // Odcinamy wiadomość na znaku nowej linii
                            
                            char msg[MAX_MSG_LEN];
                            strcpy(msg, clients[idx].buf);

                            // Przesuwamy resztę bufora na początek
                            int remaining = clients[idx].buf_len - (newline - clients[idx].buf + 1);
                            memmove(clients[idx].buf, newline + 1, remaining);
                            clients[idx].buf_len = remaining;
                            clients[idx].buf[remaining] = '\0';

                            // Przetwarzanie wyodrębnionej wiadomości
                            handle_message(idx, msg);
                        }

                        // Zabezpieczenie przed zapchaniem bufora wiadomością bez znaku nowej linii
                        if (clients[idx].buf_len == MAX_MSG_LEN - 1 && strchr(clients[idx].buf, '\n') == NULL) {
                            clients[idx].buf_len = 0; // Czyszczenie
                        }

                    } else if (size == 0 || (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                        // Klient fizycznie zerwał połączenie
                        disconnect_client(idx, epoll_fd);
                    }
                }
            }
        }
    }
    
    // Ostateczne zamknięcie zasobów (zabezpieczenie)
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1) TEMP_FAILURE_RETRY(close(clients[i].fd));
    }
    TEMP_FAILURE_RETRY(close(epoll_fd));
}

int main(int argc, char **argv) {
    if (argc != 2) usage(argv[0]);
    int timeout = atoi(argv[1]);

    // Ignorowanie zjawiska rzutu słuchawką (Złamana rura) z Etapu 4
    if (sethandler(SIG_IGN, SIGPIPE)) ERR("setting SIGPIPE");

    // Tworzenie lokalnego gniazda UNIX_SK_NAME z w7-common.h
    int listen_socket = bind_local_socket(UNIX_SK_NAME, BACKLOG);
    set_nonblock(listen_socket);

    // Odpalenie głównej pętli serwera
    doServer(listen_socket, timeout);

    // Sprzątanie gniazda
    if (TEMP_FAILURE_RETRY(close(listen_socket)) < 0) ERR("close");
    if (unlink(UNIX_SK_NAME) < 0 && errno != ENOENT) ERR("unlink");

    return EXIT_SUCCESS;
}