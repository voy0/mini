#include "w7-common.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#define BACKLOG 3
#define MAX_EVENTS 14
#define MAX_ELECTORS 7
void usage(char *name) { fprintf(stderr, "USAGE: %s socket port\n", name); exit(EXIT_FAILURE); }

typedef struct{
    int connected;
    int fd;
    int vote;
} t_electors;

int main(int argc, char **argv)
{
    char* name = argv[0];
    if(argc != 2){
        usage(name);
    }
    int tcp_listen_socket = bind_tcp_socket(atoi(argv[1]), BACKLOG);

    t_electors electors[MAX_ELECTORS]; 

    for(int i = 0; i < MAX_ELECTORS; i++){
        electors[i].connected = 0;
        electors[i].fd = -1;
        electors[i].vote = -1;
    }

    int epoll_descriptor;
    if((epoll_descriptor = epoll_create1(0)) < 0){
        ERR("epoll_create");
    }

    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = tcp_listen_socket;
    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, tcp_listen_socket, &event) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    int nfds;

    while(1){
        if ((nfds = epoll_wait(epoll_descriptor, events, MAX_EVENTS, -1)) > 0)
        {
            for (int n = 0; n < nfds; n++)
            {
                if(events[n].data.fd == tcp_listen_socket){
                    int client_socket = add_new_client(tcp_listen_socket);
                    printf("Klient polaczony\n");
                    
                    char* message = "Welcome Elector!\n";
                    if (bulk_write(client_socket, message, strlen(message)) < 0 && errno != EPIPE)
                            ERR("write:");

                    
                    struct epoll_event new_client_event;
                    new_client_event.events = EPOLLIN;
                    new_client_event.data.fd = client_socket;
                    if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, new_client_event.data.fd, &new_client_event) == -1)
                    {
                        ERR("client epoll_ctl");
                    }
                }
                else{
                    int curr_client = events[n].data.fd;
                    char data[32];
                    char msg[32];
                    memset(data, 0, sizeof(data));
                    

                    ssize_t size = read(curr_client, data, sizeof(data) - 1);
                    if(size < 0) ERR("read");
                              
                    if(size > 0){
                        int el = atoi(data);
                            
                        for(int i = 0; i < MAX_ELECTORS; i++){
                            if(electors[i].fd == curr_client){
                                if(el >= 1 && el <= 3){
                                    snprintf(msg, sizeof(msg), "Elector %d voted %d\n", i+1, el);
                                    write(curr_client, msg, sizeof(msg));
                                }
                            }
                        }
                        if(el >= 1 && el <= 7){
                            if(electors[el-1].connected == 0){
                                electors[el-1].fd = curr_client;
                                electors[el-1].connected = 1;
                                
                                snprintf(msg, sizeof(msg), "Welcome Elector of %d\n", el);
                                write(curr_client, msg, sizeof(msg));
                               
                            }
                        }
                    }
                }
            }
        }
    }

    
    if (TEMP_FAILURE_RETRY(close(tcp_listen_socket)) < 0)
        ERR("close");
}