#include "l8_common.h"
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define MAXBUF 16
#define BACKLOG 3
#define BOARD_SIZE 10

// Zmienne dla Etapu 2
#define THREAD_COUNT 3
#define FAMILIAR_DELAY 500 // w milisekundach
#define MAX_QUEUE 10

const char* spell_names[] = {"Fireball", "Ice Lance", "Arcane Missiles"};
#define SPELLS_COUNT (sizeof(spell_names) / sizeof(spell_names[0]))

void usage(char *name) { fprintf(stderr, "USAGE: %s port\n", name); }

// --- STRUKTURY DLA ETAPU 2 ---
typedef struct {
    uint16_t spell_idx;
    uint16_t x;
    uint16_t y;
} cast_command_t;

// KOLEJKA FIFO (Bufor cykliczny)
cast_command_t queue[MAX_QUEUE];
int queue_head = 0; // Skąd pobiera chowaniec
int queue_tail = 0; // Dokąd wrzuca serwer
int queue_count = 0; // Ile jest obecnie zadań w kolejce

// MECHANIZMY SYNCHRONIZACJI
pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t q_cond = PTHREAD_COND_INITIALIZER;

// Flaga bezpieczeństwa do poprawnego wyłączenia chowańców
volatile int server_running = 1;

// =========================================================
// WĄTEK CHOWAŃCA (KONSUMENT)
// =========================================================
void* familiar_thread(void* arg) {
    int id = *((int*)arg);
    free(arg); // Zwalniamy pamięć zaalokowaną dla ID

    while (1) {
        // 1. Zamknięcie kłódki dostępu do kolejki
        pthread_mutex_lock(&q_mutex);

        // 2. Jeśli kolejka jest pusta, zasypiamy na zmiennej warunkowej (brak busywaitingu!)
        while (queue_count == 0 && server_running) {
            pthread_cond_wait(&q_cond, &q_mutex);
        }

        // 3. Sprawdzenie, czy serwer nakazał ewakuację i kolejka jest już pusta
        if (!server_running && queue_count == 0) {
            pthread_mutex_unlock(&q_mutex);
            break; // Wątek kończy życie
        }

        // 4. Pobranie zadania z kolejki (FIFO)
        cast_command_t cmd = queue[queue_head];
        queue_head = (queue_head + 1) % MAX_QUEUE; // Przesunięcie "głowy" z zawinięciem
        queue_count--;

        // 5. Otwarcie kłódki (żeby serwer mógł dodawać nowe czary, podczas gdy my rzucamy ten)
        pthread_mutex_unlock(&q_mutex);

        // 6. Rzucanie zaklęcia (Czekamy FAMILIAR_DELAY poza sekcją krytyczną!)
        struct timespec ts;
        ts.tv_sec = FAMILIAR_DELAY / 1000;
        ts.tv_nsec = (FAMILIAR_DELAY % 1000) * 1000000L;
        nanosleep(&ts, NULL);

        // 7. Wypisanie informacji
        printf("[Cast] Someone casts %s onto %u,%u (Familiar %d)\n", 
               spell_names[cmd.spell_idx], cmd.x, cmd.y, id);
    }
    
    return NULL;
}


int make_socket(int domain, int type) {
    int sock;
    sock = socket(domain, type, 0);
    if (sock < 0) ERR("socket");
    return sock;
}

int bind_inet_socket(uint16_t port, int type) {
    struct sockaddr_in addr;
    int socketfd, t = 1;
    socketfd = make_socket(PF_INET, type);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t))) ERR("setsockopt");
    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) ERR("bind");
    if (SOCK_STREAM == type)
        if (listen(socketfd, BACKLOG) < 0) ERR("listen");
    return socketfd;
}

void doServer(int fd) {
    struct sockaddr_in addr;
    char buf[MAXBUF + 1];
    int messages_handled = 0;

    while(messages_handled < 4) {
        int receivedBytes;
        socklen_t size = sizeof(addr);
        if((receivedBytes = TEMP_FAILURE_RETRY(recvfrom(fd, buf, MAXBUF, 0, (struct sockaddr *)&addr, &size))) < 0)
            ERR("read:");
        buf[receivedBytes] = 0;
        char type = buf[0];

        switch(type){
            case 'l':
            {
                char* name = &buf[2];
                printf("[Login] Welcome %s", name);
                messages_handled++;
                break;
            }
            case 'c':
            {
                uint16_t* data_ptr = (uint16_t*)(&buf[2]);
                uint16_t spell_idx = ntohs(data_ptr[0]);
                // Poprawiona kolejność: pierwszy jest X, drugi jest Y wg specyfikacji.
                uint16_t x_coord = ntohs(data_ptr[1]);
                uint16_t y_coord = ntohs(data_ptr[2]);

                if (spell_idx >= SPELLS_COUNT) {
                    fprintf(stderr, "[Error] Wybor zaklecia %u poza zakresem!\n", spell_idx);
                    break;
                }

                if (x_coord >= BOARD_SIZE || y_coord >= BOARD_SIZE) {
                    fprintf(stderr, "[Error] Koordynaty (%u, %u) poza plansza!\n", x_coord, y_coord);
                    break;
                }

                // --- ETAP 2: Dodawanie do kolejki zamiast bezpośredniego wypisywania ---
                pthread_mutex_lock(&q_mutex);
                
                if (queue_count == MAX_QUEUE) {
                    fprintf(stderr, "[Error] Kolejka pelna! Odrzucono zaklecie.\n");
                } else {
                    queue[queue_tail].spell_idx = spell_idx;
                    queue[queue_tail].x = x_coord;
                    queue[queue_tail].y = y_coord;
                    
                    queue_tail = (queue_tail + 1) % MAX_QUEUE; // Cykliczne przesunięcie
                    queue_count++;

                    // Wybudzamy jednego śpiącego chowańca (Zmienna warunkowa)
                    pthread_cond_signal(&q_cond);
                }

                pthread_mutex_unlock(&q_mutex);
                messages_handled++;
                break;
            }
            case 'q':
                printf("[Quit] Someone quit. Goodbye!\n");
                messages_handled++;
                break;

            default:
                fprintf(stderr, "[Error] Nieznany typ wiadomosci: '%c'\n", type);
                break;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int fd = bind_inet_socket(atoi(argv[1]), SOCK_DGRAM);

    // 1. Uruchamianie wątków chowańców przed startem serwera
    pthread_t familiars[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        int* id = malloc(sizeof(int)); // Unikamy problemu współdzielonej referencji w pętli
        *id = i + 1;
        pthread_create(&familiars[i], NULL, familiar_thread, id);
    }

    // 2. Główna pętla serwera UDP
    doServer(fd);

    // 3. Po 4 wiadomościach: Procedura bezpiecznego wyłączania sztabu
    server_running = 0;
    
    // Budzimy WSZYSTKICH uśpionych chowańców za pomocą "broadcast", 
    // aby zorientowali się, że serwer zgasł i mogli zakończyć swoje pętle while.
    pthread_cond_broadcast(&q_cond);

    // Czekamy, aż każdy chowaniec z osobna bezpiecznie się zamknie
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(familiars[i], NULL);
    }

    if (TEMP_FAILURE_RETRY(close(fd)) < 0) ERR("close");
    
    // Niszczymy zasoby synchronizacyjne
    pthread_mutex_destroy(&q_mutex);
    pthread_cond_destroy(&q_cond);

    fprintf(stderr, "Server has terminated.\n");
    return EXIT_SUCCESS;
}