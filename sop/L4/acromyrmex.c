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

// Zmienna globalna TYLKO dla handlera potomka
volatile sig_atomic_t my_read_fd = -1;

void sigint_handler(int sig) {
    if (my_read_fd != -1) {
        close(my_read_fd); // Brutalnie urywamy rurę, żeby przerwać read()
        my_read_fd = -1;
    }
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    //fprintf(stderr, "3<=n<=20 \n");

    exit(EXIT_FAILURE);
}

void child_work(int my_id, int* fd_nodes, int V, int* my_neighbors, int my_neighbor_count) {
    
    // 1. USTAWIANIE OBSŁUGI SYGNAŁU SIGINT
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    // Zapisujemy naszą rurę do zmiennej globalnej dla handlera
    my_read_fd = fd_nodes[2 * my_id];

    // 2. SPRZĄTANIE RUR (Logika, którą przed chwilą naprawiliśmy)
    for (int k = 0; k < V; k++) {
        // Zamykam czytanie (jeśli to nie ja)
        if (k != my_id) {
            if (close(fd_nodes[2 * k]) < 0) ERR("close");
        }
        
        // Zamykam pisanie (jeśli k nie jest moim sąsiadem)
        int is_neighbor = 0;
        for (int j = 0; j < my_neighbor_count; j++) {
            if (my_neighbors[j] == k) {
                is_neighbor = 1;
                break;
            }
        }
        if (is_neighbor == 0) {
            if (close(fd_nodes[2 * k + 1]) < 0) ERR("close");
        }
    }

    // 3. PĘTLA NASŁUCHUJĄCA
    char buffer[256]; // Tymczasowy bufor, w Etapie 3 zamienimy go na strukturę mrówki
    int bytes_read;
    
    // Czytamy tak długo, aż przeczytamy 0 (wszyscy zginęli) lub zepsuje się rura (nasz SIGINT)
    while ((bytes_read = read(my_read_fd, buffer, sizeof(buffer))) > 0) {
        // Tutaj w Etapie 3 węzeł będzie obsługiwał mrówkę!
        // Na razie pętla po prostu zjada dane i czeka dalej.
    }

    // Jeśli wyrzuciło nas z pętli przez błąd inny niż celowe zamknięcie rury lub sygnał
    if (bytes_read < 0 && errno != EBADF && errno != EINTR) {
        ERR("read");
    }

    // 4. ZWALNIANIE ZASOBÓW (Zgodnie z poleceniem)
    // Zamykamy końcówki do pisania do naszych sąsiadów, żeby nie zostawić śmieci
    for (int j = 0; j < my_neighbor_count; j++) {
        close(fd_nodes[2 * my_neighbors[j] + 1]);
    }
    
    printf("Węzeł %d: Zwalniam zasoby i ewakuuję się z dżungli!\n", my_id);
    exit(EXIT_SUCCESS);
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
    fclose(f);

    int fd_nodes[2*V];
    for(int i = 0; i < V; i++){
        if(pipe(&fd_nodes[2*i]) < 0) ERR("pipe");
    }

    for(int i = 0; i < V; i++){
        switch(fork()){
            case 0:
                printf("%d: ", i);
                for(int j = 0; j < neighbor_count[i]; j++){
                    printf("%d ", neighbors[i][j]);
                }
                printf("\n");
                child_work(i, fd_nodes, V, neighbors[i], neighbor_count[i]);

                exit(EXIT_SUCCESS);

            case -1:
                ERR("fork");
        }
    }

    
    while(wait(NULL) > 0);
}
