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

    while (do_work) {
        // Czekamy na pakiet UDP. W UDP nie obchodzi nas kto go wysłał, więc parametry adresu to NULL
        ssize_t read_bytes = recvfrom(sock, buf, MSG_MAX, 0, NULL, NULL);
        
        if (read_bytes < 0) {
            if (errno == EINTR) {
                continue; // Przerwanie sygnałem, pętla sprawdzi do_work i ewentualnie się zakończy
            }
            ERR("recvfrom");
        }

        // Minimalna długość wiadomości to 24 bajty (16 bajtów loginu + 8 bajtów polecenia)
        if (read_bytes < 24) {
            printf("error: wrong message length %ld\n", read_bytes);
            continue;
        }

        // --- 1. PARSOWANIE LOGINU ---
        // Tworzymy bufor o wielkości 17 bajtów wyzerowany na starcie, 
        // aby zagwarantować znak końca łańcucha '\0', nawet gdy login zajmuje pełne 16 bajtów.
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

        // --- 2. PARSOWANIE KOMENDY ---
        char cmd[9] = {0};
        memcpy(cmd, buf + 16, 8);

        // --- 3. WALIDACJA KOMENDY I PARAMETRÓW ---
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
                do_work = 0; // Komenda EXIT kończy działanie programu
            }
        } 
        else if (strcmp(cmd, "COMPUTE") == 0) 
        {
            // COMPUTE musi mieć parametry (więc > 24) i muszą to być pary liczb 4-bajtowych 
            // (1 para = 8 bajtów, więc reszta z dzielenia przez 8 musi wynosić 0)
            if (read_bytes <= 24 || (read_bytes - 24) % 8 != 0) {
                printf("error: wrong message length %ld\n", read_bytes);
                continue;
            }

            printf("%s: %s ", login, cmd);

            int num_pairs = (read_bytes - 24) / 8;
            for (int i = 0; i < num_pairs; i++) {
                uint32_t a, b;
                // Kopiujemy bajty parami. Bezpieczne odczytywanie z pamięci, by uniknąć problemów z wyrównaniem.
                memcpy(&a, buf + 24 + (i * 8), 4);
                memcpy(&b, buf + 24 + (i * 8) + 4, 4);

                // Konwersja z Network Byte Order do Host Byte Order
                printf("(%u, %u) ", ntohl(a), ntohl(b));
            }
            printf("\n");
        } 
        else 
        {
            printf("error: unknown command %s\n", cmd);
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}