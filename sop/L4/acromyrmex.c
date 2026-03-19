#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>


#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), exit(EXIT_FAILURE))

#define MAX_GRAPH_NODES 100

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "3<=n<=20 \n");

    exit(EXIT_FAILURE);
}

void child_work(int* neighbors, int* neighbor_count){
    
}

int main(int argc, char** argv) {
    if (argc != 4) {
        usage(argv[0]);
    }

    FILE* f = fopen(argv[1], "r");
    if(f == NULL) ERR("fopen");

    int V;
    if(fscanf(f, "%d", &V) != 1) ERR("fscanf");


    int neighbors[MAX_GRAPH_NODES][MAX_GRAPH_NODES];
    int neighbor_count[MAX_GRAPH_NODES] = {0};

    int u, v;
    while (fscanf(f, "%d %d", &u, &v) == 2) {
        neighbors[u][neighbor_count[u]] = v;
        neighbor_count[u]++;
    }

    int fd_nodes[2*V];
    for(int i = 0; i < 2*V; i++){
        if(pipe(&fd_nodes[2*i]) < 0) ERR("pipe");
    }

    int* fd_neighbors;


    for(int i = 0; i < V; i++){
        switch(fork()){
            case 0:
                printf("%d: ", i);
                for(int j = 0; j < neighbor_count[i]; j++){
                    printf("%d ", neighbors[i][j]);
                }
                printf("\n");
                
                fd_neighbors = malloc(2*neighbor_count[i]*sizeof(int));

                for(int j = 0; j < neighbor_count[i]; j++){
                    if(pipe(&fd_neighbors[2*j]) < 0) ERR("pipe");
                }

                child_work(neighbors, neighbor_count);

                exit(EXIT_SUCCESS);

            case -1:
                ERR("fork");
        }
    }

    fclose(f);
    while(wait(NULL) > 0);
}
