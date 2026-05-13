#include "w7-common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define BACKLOG 1
#define MAX_EVENTS 12
void usage(char *name) { fprintf(stderr, "USAGE: %s socket port\n", name); exit(EXIT_FAILURE);}

int check_elector(char* data){
    int res = atoi(data);
    if(res < 1 || res > 7)
        return -1;
    return res;
}

void doServer(int tcp_listen_socket){
    // int new_flags = fcntl(tcp_listen_socket, F_GETFL) | O_NONBLOCK;
    // fcntl(tcp_listen_socket, F_SETFL, new_flags);

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
    int client_state[1024];
    int present_electors[7];
    int electors_votes[7];
    memset(present_electors, 0, sizeof(present_electors));
    memset(electors_votes, 0, sizeof(electors_votes));
    memset(client_state, 0, sizeof(client_state));

    // for(int i = 0; i < 7; i++){
    //     for(int j = 0; j < 3; j++){
    //         electors_votes[i][j] = 0;
    //     }
    // }
    while(1){
        if ((nfds = epoll_wait(epoll_descriptor, events, MAX_EVENTS, -1)) > 0)
        {
            for (int n = 0; n < nfds; n++)
            {
                if(events[n].data.fd  == tcp_listen_socket)
                {
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
                    memset(data, 0, sizeof(data));

                    ssize_t size = read(curr_client, data, sizeof(data) - 1);
                    
                    if(size > 0){
                        int res = check_elector(data);
                        int flag = 0;

                        if(client_state[curr_client] > 0){
                            int elector = client_state[curr_client];
                            int vote = atoi(data);
                            if(vote < 1 || vote > 3){
                                printf("Incorrect vote from elector %d\n", elector);
                                continue;
                            }
                            electors_votes[elector-1] = vote;
                            printf("Elector %d voted for %d\n", elector, vote);
                        }
                        else if(res < 0 || present_electors[res-1] > 0)
                        {
                            flag++;
                        }
                        else{
                            present_electors[res-1] = curr_client;
                            client_state[curr_client] = res;
                            printf("[S<-%d]: %s\n", curr_client, data);
                            char message[32];
                            snprintf(message, sizeof(message), "Welcome elector of %d\n", res);
                            if (bulk_write(curr_client, message, strlen(message)) < 0 && errno != EPIPE)
                                ERR("write:");  
                            
                            
                        }
                        if(flag > 0){
                            // if(res > 0){
                            //     present_electors[res-1] = 0;
                            // }
                            printf("[S] incorrect elector\n");
                            epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, curr_client, NULL);
                            if (TEMP_FAILURE_RETRY(close(curr_client)) < 0)
                                ERR("close");
                        }
                        
                    }
                    else if (size == 0 || (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                        printf("[S]: Klient (FD: %d) się rozłączył.\n", curr_client);

                        if (client_state[curr_client] > 0) {
                            int elector_id = client_state[curr_client];
                            present_electors[elector_id - 1] = 0; 
                            printf("[S]: Zwolniono miejsce dla elektora %d\n", elector_id);
                        }

                        client_state[curr_client] = 0;

                        epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, curr_client, NULL);

                        if (TEMP_FAILURE_RETRY(close(curr_client)) < 0)
                            ERR("close");
                    }
                    
                }
            }
        }
    }


    
}

int main(int argc, char **argv)
{
    char* name = argv[0];
    if(argc != 2){
        usage(name);
    }
    int tcp_listen_socket = bind_tcp_socket(atoi(argv[1]), BACKLOG);
    
    doServer(tcp_listen_socket);

    if(TEMP_FAILURE_RETRY(close(tcp_listen_socket))< 0) ERR("close");
    fprintf(stderr, "Server has terminated.\n");
    return EXIT_SUCCESS;
}
