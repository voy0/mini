#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))



void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "2<n<=10 - number of children\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv){
    
    char* name = argv[0];
    if(argc != 2){
        usage(name);
    }

    int n = atoi(argv[1]);
    if(n < 2 || n > 10){
        usage(name);
    }

    int* fds;
 
    fds = malloc(sizeof(int)*2*(n + 1));
    for(int i = 0; i < n+1; i++){
        pipe(&fds[2*i]);
    }

    for(int i = 0; i < n; i++){
        int s = fork();
        switch(s){
            case 0:{
                srand(getpid());
                for(int j = 0; j < 2*(n+1); j++){
                    if(2*i != j && 2*i + 3 != j){
                        close(fds[j]);
                    }
                }
                while(1){
                    char buf[5];
                    int broken = read(fds[i*2], buf, sizeof(buf));
                    if(broken < 0)
                        ERR("read");
                    if(broken == 0){
                        fprintf(stderr, "%d: TERMINATING...\n", getpid());

                        break;
                    }
                    int num;
                    sscanf(buf, "%d",  &num);
                    printf("%d: received %d\n", getpid(), num);

                    if(num == 0){
                        fprintf(stderr, "%d: received 0 TERMINATING...\n", getpid());

                        break;
                    }

                    num += rand()%21 - 10;
                    snprintf(buf, sizeof(buf), "%d", num);
                    if(write(fds[i*2 + 3], buf, sizeof(buf))< 0)
                        ERR("write");
                }
                close(fds[2*i]);
                close(fds[2*i+3]);
                exit(EXIT_SUCCESS);
            }
            case -1:{
                ERR("fork");
            }
        }
    }
    for(int j = 0; j < 2*(n+1); j++){
        if(j != 1 && j != n*2){
            close(fds[j]);
        }
    }
    
    while(1){     
        char buf[5];
        snprintf(buf, sizeof(buf), "%d", 1);
        printf("PARENT: sent %d\n", 1);
        if(write(fds[1], buf, sizeof(buf)) < 0)
            ERR("write");

        memset(buf, 0, sizeof(buf));
        sleep(1);
        int broken = read(fds[n*2], buf, sizeof(buf));
        if(broken < 0)
            ERR("read");
        if(broken == 0)
        {
            fprintf(stderr, "PARENT: broken pipe, TERMINATING...\n");
            break;
        }
        int num;
        sscanf(buf, "%d", &num);
        printf("PARENT: received %d\n", num);
    }
    close(fds[1]);
    close(fds[n*2]);
    while(wait(NULL) > 0)
        {}
    free(fds);
    
    exit(EXIT_SUCCESS);
}