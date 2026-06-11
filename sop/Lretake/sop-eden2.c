#include "l8_common.h"

void usage(char* name)
{
    printf("%s <in_port>\n", name);
    printf("  in_port - port that accepts messages\n");
    exit(EXIT_FAILURE);
}

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig) {
    do_work = 0;
}

typedef struct task {
    char login[17];
    uint32_t count;
    uint32_t seed;
    struct task* next;
} task_t;

typedef struct {
    task_t* head;
    task_t* tail;
    pthread_mutex_t mutex;
} task_queue_t;

// Globalna kolejka zadań
task_queue_t task_queue = {NULL, NULL, PTHREAD_MUTEX_INITIALIZER};

int main(int argc, char** argv) { 
    if (argc != 2) {
        usage(argv[0]);
    }

    uint16_t port = (uint16_t)atoi(argv[1]);
    
    // Ustawiamy obsługę sygnału, aby móc grzecznie zamknąć gniazdo
    sethandler(sigint_handler, SIGINT);

    // Wykorzystujemy dostarczoną funkcję z biblioteki dla protokołu UDP (SOCK_DGRAM)
    int sock = bind_inet_socket(port, SOCK_DGRAM, 0);
    char buf[MSG_MAX];

    // Struktury do adresowania zwrotnego (dla komendy LIST)
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (do_work) {
        // POBIERANIE ADRESU NADAWCY JEST KLUCZOWE
        ssize_t read_bytes = recvfrom(sock, buf, MSG_MAX, 0, (struct sockaddr*)&client_addr, &client_len);
        
        if (read_bytes < 0) {
            if (errno == EINTR) continue;
            ERR("recvfrom");
        }

        if (read_bytes < 24) {
            printf("error: wrong message length %ld\n", read_bytes);
            continue;
        }

        char login[17] = {0};
        memcpy(login, buf, 16);

        bool user_found = false;
        for (int i = 0; i < USERS; i++) {
            if (strcmp(login, LOGINS[i]) == 0) {
                user_found = true;
                break;
            }
        }

        if (!user_found) {
            printf("error: unknown user %s\n", login);
            continue;
        }

        char cmd[9] = {0};
        memcpy(cmd, buf + 16, 8);

        if (strcmp(cmd, "EXIT") == 0 || strcmp(cmd, "RUN") == 0 || 
            strcmp(cmd, "PAUSE") == 0 || strcmp(cmd, "LIST") == 0 || 
            strcmp(cmd, "GATHER") == 0) 
        {
            if (read_bytes != 24) {
                printf("error: wrong message length %ld\n", read_bytes);
                continue;
            }
            
            printf("%s: %s\n", login, cmd);
            
            if (strcmp(cmd, "EXIT") == 0) {
                do_work = 0;
            }
            else if (strcmp(cmd, "LIST") == 0) {
                // --- OBSŁUGA KOMENDY LIST ---
                
                // Modyfikacja portu zwrotnego: port nadawcy + 1
                uint16_t client_port = ntohs(client_addr.sin_port);
                client_addr.sin_port = htons(client_port + 1);

                // Bufor wyjściowy może pomieścić maksymalnie MSG_MAX bajtów (64 bajty)
                // 1 para to 8 bajtów (2 x uint32_t), więc zmieści się 8 par na pakiet.
                uint32_t out_buf[MSG_MAX / sizeof(uint32_t)];
                int out_idx = 0;

                pthread_mutex_lock(&task_queue.mutex);
                task_t* curr = task_queue.head;
                
                while (curr != NULL) {
                    if (strcmp(curr->login, login) == 0) {
                        // Pakujemy liczby, przywracając Network Byte Order dla klienta
                        out_buf[out_idx++] = htonl(curr->count);
                        out_buf[out_idx++] = htonl(curr->seed);

                        // Jeżeli bufor jest pełny (osiągnęliśmy limit MSG_MAX)
                        if (out_idx * sizeof(uint32_t) == MSG_MAX) {
                            if (sendto(sock, out_buf, MSG_MAX, 0, (struct sockaddr*)&client_addr, client_len) < 0) {
                                perror("sendto"); // Nie ubijamy serwera przez błąd wysyłki
                            }
                            out_idx = 0; // Resetujemy bufor dla kolejnej paczki
                        }
                    }
                    curr = curr->next;
                }
                
                // Wysyłamy ewentualne "resztki" z bufora, które nie dobiły do 64 bajtów
                if (out_idx > 0) {
                    if (sendto(sock, out_buf, out_idx * sizeof(uint32_t), 0, (struct sockaddr*)&client_addr, client_len) < 0) {
                        perror("sendto final packet");
                    }
                }
                pthread_mutex_unlock(&task_queue.mutex);
            }
        } 
        else if (strcmp(cmd, "COMPUTE") == 0) 
        {
            if (read_bytes <= 24 || (read_bytes - 24) % 8 != 0) {
                printf("error: wrong message length %ld\n", read_bytes);
                continue;
            }

            printf("%s: %s ", login, cmd);

            int num_pairs = (read_bytes - 24) / 8;
            for (int i = 0; i < num_pairs; i++) {
                uint32_t count_net, seed_net;
                memcpy(&count_net, buf + 24 + (i * 8), 4);
                memcpy(&seed_net, buf + 24 + (i * 8) + 4, 4);

                uint32_t count = ntohl(count_net);
                uint32_t seed = ntohl(seed_net);

                printf("(%u, %u) ", count, seed);

                // --- OBSŁUGA KOMENDY COMPUTE ---
                if (count > 10000000) {
                    printf("\nerror: count %u exceeds maximum allowed samples (10000000)", count);
                    continue; // Zignoruj tylko to konkretne zadanie, kontynuuj parsowanie reszty
                }

                // Alokacja nowego węzła listy
                task_t* new_task = malloc(sizeof(task_t));
                if (!new_task) ERR("malloc");
                
                strcpy(new_task->login, login);
                new_task->count = count;
                new_task->seed = seed;
                new_task->next = NULL;

                // Bezpieczne dodawanie na koniec kolejki (O(1))
                pthread_mutex_lock(&task_queue.mutex);
                if (task_queue.tail == NULL) {
                    task_queue.head = task_queue.tail = new_task;
                } else {
                    task_queue.tail->next = new_task;
                    task_queue.tail = new_task;
                }
                pthread_mutex_unlock(&task_queue.mutex);
            }
            printf("\n");
        } 
        else 
        {
            printf("error: unknown command %s\n", cmd);
        }
    }

    // --- SPRZĄTANIE PAMIĘCI PRZED WYJŚCIEM ---
    task_t* curr = task_queue.head;
    while (curr != NULL) {
        task_t* to_delete = curr;
        curr = curr->next;
        free(to_delete);
    }
    
    close(sock);
    return EXIT_SUCCESS;
}