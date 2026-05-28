#include "l8_common.h"
#include <stdint.h>
#define MAXBUF 16
#define BACKLOG 3

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
        
        // uint16_t* data_ptr = (uint16_t*)(&buf[0]);
        // uint16_t x = ntohs(data_ptr[0]);
        // uint16_t y = ntohs(data_ptr[1]);
        // uint16_t p = ntohs(data_ptr[2]);
        // int offset = 3* sizeof(uint16_t);
        // char* name = buf + offset;

        int x, y, p;
        char nazwa[129]; 
        memset(nazwa, 0, sizeof(nazwa));
        int parsed_items = sscanf(buf, "%d %d %d %[^\n]", &x, &y, &p, nazwa);
        
        if (parsed_items != 4 || x < 0 || x > 99 || y < 0 || y > 99 || (p != 0 && p != 1)) {
            fprintf(stderr, "[!] Odrzucono: Zle sformatowany meldunek od zwiadowcow.\n");
        } else {
            // SUKCES
            if (p == 1) {
                printf("Nasz oddzial %s byl widziany na pozycji %d:%d\n", nazwa, x, y);
            } else {
                printf("Wrogi oddzial %s byl widziany na pozycji %d:%d\n", nazwa, x, y);
            }
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