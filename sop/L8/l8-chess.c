#include "l8_common.h"
#include <stdint.h>
#define MAXBUF 16
#define BACKLOG 3
#define BOARD_SIZE 10
const char* spell_names[] = {"Fireball", "Ice Lance", "Arcane Missiles"};
#define SPELLS_COUNT (sizeof(spell_names) / sizeof(spell_names[0]))

#define PACKET_SIZE 16
void usage(char *name) { fprintf(stderr, "USAGE: %s port\n", name); }

int make_socket(int domain, int type)
{
    int sock;
    sock = socket(domain, type, 0);
    if (sock < 0)
        ERR("socket");
    return sock;
}

int bind_inet_socket(uint16_t port, int type)
{
    struct sockaddr_in addr;
    int socketfd, t = 1;
    socketfd = make_socket(PF_INET, type);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
        ERR("setsockopt");
    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        ERR("bind");
    if (SOCK_STREAM == type)
        if (listen(socketfd, BACKLOG) < 0)
            ERR("listen");
    return socketfd;
}
// struct __attribute__((__packed__)) udp_packet {
//     char type;          // 1 bajt ('l', 'c', 'q')
//     char padding;       // 1 bajt (ignorowany)
    
//     union {
//         char name[14];             // Interpretacja dla wiadomości 'l'
//         uint16_t cast_data[3];     // Interpretacja dla wiadomości 'c' [0]=spell, [1]=X, [2]=Y
//     } payload;
// };
void doServer(int fd){
    struct sockaddr_in addr;
    char buf[MAXBUF + 1];
    int messages_handled = 0;
    while(messages_handled < 4){
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
                break;
            }
            case 'c':
            {
                uint16_t* data_ptr = (uint16_t*)(&buf[2]);
                uint16_t spell_idx = ntohs(data_ptr[0]);
                uint16_t y_coord = ntohs(data_ptr[1]);
                uint16_t x_coord = ntohs(data_ptr[2]);

                if (spell_idx >= SPELLS_COUNT) {
                    fprintf(stderr, "[Error] Wybor zaklecia %u poza zakresem!\n", spell_idx);
                    break;
                }

                // WALIDACJA 3: Koordynaty na planszy (0 do 9)
                if (x_coord >= BOARD_SIZE || y_coord >= BOARD_SIZE) {
                    fprintf(stderr, "[Error] Koordynaty (%u, %u) poza plansza!\n", x_coord, y_coord);
                    break;
                }

                printf("[Cast] Someone casts %s onto %u,%u\n", spell_names[spell_idx], x_coord, y_coord);
                break;
            }

            case 'q':
                // Skoro to 'q', reszta to puste znaki i nie interesują nas.
                printf("[Quit] Someone quit. Goodbye!\n");
                break;

            default:
                // WALIDACJA 4: Zły znak na początku
                fprintf(stderr, "[Error] Nieznany typ wiadomosci: '%c'\n", type);
                break;
        }
        messages_handled++;
    }
}

int main(int argc, char **argv)
{
    int fd;
    if (argc != 2)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    fd = bind_inet_socket(atoi(argv[1]), SOCK_DGRAM);
    doServer(fd);
    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
    fprintf(stderr, "Server has terminated.\n");
    return EXIT_SUCCESS;
}