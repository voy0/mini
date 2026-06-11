#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define MAX_GRAPH_NODES 32
#define MAX_PATH_LENGTH (2 * MAX_GRAPH_NODES)

#define FIFO_NAME "/tmp/colony_fifo"

volatile sig_atomic_t close_fds = -1;

int set_handler(void (*f)(int), int sig)
{
    struct sigaction act = {0};
    act.sa_handler = f;
    if (sigaction(sig, &act, NULL) == -1)
        return -1;
    return 0;
}

void sigint_handler(int sig){
    if(close_fds != -1){
        close(close_fds);
        close_fds = -1;
    }
}

void msleep(int ms)
{
    struct timespec tt;
    tt.tv_sec = ms / 1000;
    tt.tv_nsec = (ms % 1000) * 1000000;
    while (nanosleep(&tt, &tt) == -1)
    {
    }
}

void usage(int argc, char* argv[])
{
    printf("%s graph start dest\n", argv[0]);
    printf("  graph - path to file containing colony graph\n");
    printf("  start - starting node index\n");
    printf("  dest - destination node index\n");
    exit(EXIT_FAILURE);
}
typedef struct{
    int id;
    int path[MAX_PATH_LENGTH];
    int path_lenght;
}ANT;
int main(int argc, char* argv[])
{
    set_handler(sigint_handler, SIGINT);
    if (argc != 4)
        usage(argc, argv);

    int start = atoi(argv[2]);
    int dest = atoi(argv[3]);

    int fifo;

    if (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0)
        if (errno != EEXIST)
            ERR("create fifo");
    if ((fifo = open(FIFO_NAME, O_RDONLY | O_NONBLOCK)) < 0)
        ERR("open");

    // int new_flags = fcntl(fifo, F_GETFL) | O_NONBLOCK;
    // fcntl(fifo, F_SETFL, new_flags);

    FILE* f = fopen(argv[1], "r");
    if(f == NULL) ERR("fopen");

    int V;
    if(fscanf(f, "%d", &V) != 1) ERR("fscanf");
    if(start >= V || dest >= V){
        ERR("wrong index");
    }
    int G[V][V];

    int u, v;
    for(int i = 0; i < V; i++){
        for(int j = 0; j < V; j++){
            G[i][j] = 0;
        }
    }
    
    while(fscanf(f, "%d %d", &u, &v) == 2){
        G[u][v] = 1;
    }
    fclose(f);

    int fds[V*2];
    for(int i = 0; i < V; i++){
        pipe(&fds[2*i]);
    }
    for(int i = 0; i < V; i++){
        int f = fork();
        switch(f){
            case 0:{
                
                close(fifo);
                int write_fifo = -1;
                if(i == dest){
                    write_fifo = open(FIFO_NAME, O_WRONLY);
                    if(write_fifo < 0){
                        ERR("open fifo");
                    }
                }

                srand(getpid());
                printf("%d: ", i);
                for(int j = 0; j < V; j++){
                    if(G[i][j] == 1){
                        printf(" [%d] ",j);
                    }
                    else{                   
                        close(fds[2*j+1]);
                    }
                    if(j != i){

                        close(fds[2*j]);
                    }
                }
                close_fds = 2*i;
                
                printf("\n");
                while(close_fds != -1){
                    ANT received_ant;
                    msleep(100);
                    int r = read(fds[2*i], &received_ant, sizeof(ANT));
                    if(r > 0){
                        if(dest == i){
                            printf("%d: Ant [%d] found food\n", i, received_ant.id);
                            if(write(write_fifo, &received_ant, sizeof(ANT)) < 0){
                                ERR("write");
                            }
                            continue;
                        }

                        int neighbors[MAX_GRAPH_NODES];
                        int neighbor_count = 0;
                        for(int j = 0; j < V; j++){
                            if(G[i][j] == 1){
                                neighbors[neighbor_count++] = j;
                            }
                        }

                        if(received_ant.path_lenght >= MAX_PATH_LENGTH || neighbor_count == 0){
                            printf("%d: Ant [%d] got lost\n", i, received_ant.id);
                        }
                        else{ 
                            int r = rand()%neighbor_count;
                            int n = neighbors[r];

                            received_ant.path[received_ant.path_lenght++] = i;
                            printf("%d: received ant [%d]\n", i, received_ant.id);
                            if(write(fds[2*n+1], &received_ant, sizeof(ANT)) < 0){
                                ERR("write");
                            }
                        }
                    }
                    else if(r < 0){
                        if(errno == EINTR){
                            continue;
                        }
                        ERR("read");
                    }
                    else{
                        break;
                    }


                }
                exit(EXIT_SUCCESS);
            }
            case -1:{
                ERR("fork");
            }
        }
    }

    for(int i = 0; i < 2*V; i++){
        if(i != 2*start+1 && i != 2*dest){
            close(fds[i]);
        }
    }

    int id = 100;
    while(1){
        sleep(1);
        ANT new_ant;
        new_ant.id = id++;
        new_ant.path_lenght = 0;
        printf("P: sent ant [%d]\n", new_ant.id);
        if(write(fds[2*start+1], &new_ant, sizeof(ANT)) < 0){
            ERR("write");
        }
        ANT fed_ant;
        if(read(fifo, &fed_ant, sizeof(ANT)) > 0){
            printf("P: Ant [%d] path:", fed_ant.id);
            for(int i = 0; i < fed_ant.path_lenght; i++){
                printf(" %d ", fed_ant.path[i]);
            }
            printf("\n");
        }
    }

    for(int i = 0; i < V; i++) {
        wait(NULL);
    }

    exit(EXIT_SUCCESS);
}