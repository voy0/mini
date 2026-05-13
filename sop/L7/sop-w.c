#include "w7-common.h"
#define MAX_EVENTS 10
#define WElCOME_MSG "Welcome elector\n"

void usage(char* program_name)
{
    fprintf(stderr, "Usage: socket_id\n");
    exit(EXIT_FAILURE);
}

void do_server(int sfd){
    int epoll_fd = epoll_create1(0);
    if(epoll_fd < 0) ERR("epoll_create1");

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sfd;
    struct epoll_event events[MAX_EVENTS];
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &ev)) ERR("epoll_ctl");

    while(1){
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if(nfds < 0) ERR("epoll_wait");
        for(int i = 0; i < nfds; i++){
            int fd = events[i].data.fd;
            if(fd == sfd){ // socket fd - new client
                int client_fd = add_new_client(sfd);
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;

                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev)) ERR("epoll_ctl");

                if(write(client_fd, WElCOME_MSG, sizeof(WElCOME_MSG)) < 0) ERR("write");

                int flags = fcntl(client_fd, F_GETFL, 0) |O_NONBLOCK;
                fcntl(client_fd, F_SETFL, flags);
            }
            else{ // client messaged us
                char buf[1024];
                int n = read(fd, buf, sizeof(buf) - 1);
                if(n < 0) ERR("read");
                if(n == 0){
                    printf("client disconnected\n");
                    close(fd);
                }
                buf[n] = '\0';
                puts(buf);
            }
        }
    }
}

int main(int argc, char**argv){
    if(argc != 2)
        usage(argv[0]);

    uint16_t port = atoi(argv[1]);
    int sfd = bind_tcp_socket(port, 10);
    if(sfd < 0) ERR("bind_tcp_socke");

    int client_fd = add_new_client(sfd);

    do_server(client_fd);

    printf("New client connected\n");
    close(sfd);
    close(client_fd);
}