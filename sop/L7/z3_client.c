#include "w7-common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define MAX_EVENTS 2 // Potrzebujemy tylko 2: Klawiatura (STDIN) i Serwer (Socket)
#define MAX_CITIES 20

// Zmienna globalna do bezpiecznego wychodzenia po SIGINT
volatile sig_atomic_t do_work = 1;
void sigint_handler(int sig) { do_work = 0; }

void usage(char *name) {
    fprintf(stderr, "USAGE: %s ip port\n", name);
    exit(EXIT_FAILURE);
}

// Funkcja drukująca raport o własności miast (komenda 'o' oraz koniec programu)
void print_cities(char *cities) {
    printf("\n=== STAN TERYTORIUM (MAPA ZWIADOWCY) ===\n");
    for (int i = 0; i < MAX_CITIES; i++) {
        printf("Miasto %02d: ", i + 1);
        if (cities[i] == 'g') printf("Grecy\n");
        else if (cities[i] == 'p') printf("Persowie\n");
        else printf("Nieznany (Mgla wojny)\n");
    }
    printf("========================================\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
    }

    // Inicjalizacja ziarna losowania dla komendy 't'
    srand(time(NULL)); 

    // Ochrona klienta przed sygnałami
    if (sethandler(SIG_IGN, SIGPIPE)) ERR("setting SIGPIPE");
    if (sethandler(sigint_handler, SIGINT)) ERR("setting SIGINT");

    // Tablica stanu miast - na początku wszystkie są NIEZNANE ('u')
    char cities[MAX_CITIES];
    for (int i = 0; i < MAX_CITIES; i++) cities[i] = 'u';

    // Nawiązanie połączenia z serwerem
    int socket_fd = connect_tcp_socket(argv[1], argv[2]);
    printf("[K]: Polaczono z biblioteka w Sparcie (%s:%s)\n", argv[1], argv[2]);
    printf("---------------------------------------\n");
    printf("Instrukcje zwiadowcy:\n");
    printf("  e      - ewakuacja (wyjscie z programu)\n");
    printf("  m XXX  - wyslij recznie wiadomosc z 3 znakow\n");
    printf("  t XX   - travel (losuj wlasciciela dla miasta XX)\n");
    printf("  o      - owners (wyswietl aktualna mape miast)\n");
    printf("---------------------------------------\n");

    // =========================================================
    // INICJALIZACJA EPOLL DLA KLIENTA
    // =========================================================
    int epoll_descriptor;
    if ((epoll_descriptor = epoll_create1(0)) < 0) ERR("epoll_create");

    struct epoll_event ev_stdin, ev_sock, events[MAX_EVENTS];

    // Kamera 1: Klawiatura użytkownika (STDIN)
    ev_stdin.events = EPOLLIN;
    ev_stdin.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, STDIN_FILENO, &ev_stdin) == -1) ERR("epoll stdin");

    // Kamera 2: Serwer biblioteki
    ev_sock.events = EPOLLIN;
    ev_sock.data.fd = socket_fd;
    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, socket_fd, &ev_sock) == -1) ERR("epoll sock");

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    while (do_work) {
        int nfds = epoll_pwait(epoll_descriptor, events, MAX_EVENTS, -1, &oldmask);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            ERR("epoll_pwait");
        }

        for (int n = 0; n < nfds; n++) {
            
            // ========================================================
            // ZDARZENIE 1: ZWIADOWCA WPISUJE KOMENDĘ NA KLAWIATURZE
            // ========================================================
            if (events[n].data.fd == STDIN_FILENO) {
                char buf[256];
                memset(buf, 0, sizeof(buf));
                ssize_t size = read(STDIN_FILENO, buf, sizeof(buf) - 1);
                
                if (size <= 0) continue;

                if (buf[0] == 'e') {
                    do_work = 0; // Komenda (e)xit
                } 
                else if (buf[0] == 'o') {
                    print_cities(cities); // Komenda (o)wners
                } 
                else if (buf[0] == 'm' && buf[1] == ' ' && size >= 5) {
                    // Komenda (m XXX) - Wysyłamy dowolne 3 znaki + \n
                    char msg[4];
                    msg[0] = buf[2];
                    msg[1] = buf[3];
                    msg[2] = buf[4];
                    msg[3] = '\n';
                    if (bulk_write(socket_fd, msg, 4) < 0 && errno != EPIPE) ERR("write");
                } 
                else if (buf[0] == 't' && buf[1] == ' ') {
                    // Komenda (t XX) - Travel
                    int city = atoi(&buf[2]);
                    if (city >= 1 && city <= MAX_CITIES) {
                        // Losujemy g (Grecy) lub p (Persowie)
                        char faction = (rand() % 2 == 0) ? 'g' : 'p';
                        
                        // Zapisujemy na lokalnej mapie zwiadowcy
                        cities[city - 1] = faction;
                        
                        // Konstruujemy i wysyłamy format YXX\n (np. p04\n)
                        char msg[5];
                        snprintf(msg, sizeof(msg), "%c%02d\n", faction, city);
                        if (bulk_write(socket_fd, msg, 4) < 0 && errno != EPIPE) ERR("write");
                        
                        printf("[LOKALNIE]: Miasto %02d nalezace do %s zaraportowane do Sparty.\n", 
                               city, faction == 'g' ? "Grekow" : "Persow");
                    } else {
                        printf("[Błąd]: Numer miasta musi byc z zakresu [1, 20]\n");
                    }
                }
            } 
            // ========================================================
            // ZDARZENIE 2: CENTRALNA BIBLIOTEKA WYSYŁA WIEŚCI
            // ========================================================
            else if (events[n].data.fd == socket_fd) {
                char msg[5];
                memset(msg, 0, sizeof(msg));
                ssize_t size = read(socket_fd, msg, 4);

                // Obsługa utraty połączenia (Etap 4)
                if (size == 0 || (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    printf("\n[!] Serwer w Sparcie zostal zamkniety/odlaczony.\n");
                    do_work = 0; // Serwer padł, zwiadowca kończy pracę
                } 
                else if (size == 4) {
                    // Serwer przysyła dane w takim samym formacie - YXX\n
                    if ((msg[0] == 'g' || msg[0] == 'p') && msg[3] == '\n') {
                        int city = (msg[1] - '0') * 10 + (msg[2] - '0');
                        if (city >= 1 && city <= MAX_CITIES) {
                            // Aktualizujemy lokalną mapę danymi od innych zwiadowców!
                            cities[city - 1] = msg[0];
                            printf("\n[WIESCI ZE SPARTY]: Miasto %02d zostalo przejete przez: %s!\n", 
                                   city, msg[0] == 'g' ? "Grekow" : "Persow");
                        }
                    }
                }
            }
        }
    }

    // ========================================================
    // SPRZĄTANIE (Po komendzie 'e' lub wciśnięciu Ctrl+C)
    // ========================================================
    print_cities(cities); // Zgodnie z poleceniem, przed wyjściem wypisujemy stan

    // Zwolnienie zasobów
    epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, STDIN_FILENO, NULL);
    epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, socket_fd, NULL);
    TEMP_FAILURE_RETRY(close(epoll_descriptor));
    TEMP_FAILURE_RETRY(close(socket_fd));
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    
    printf("\n[K]: Zwiadowca zakonczyl prace.\n");
    return EXIT_SUCCESS;
}