#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))



void child_work(int fdr, int fdw){
    char buf[20];
    srand(getpid());

    while(1){

        int broken = read(fdr, buf, 20);

        if(broken < 0)  ERR("read");
        if(broken == 0){
            printf("\n%d: ZERWANOPIPE \n", getpid());
            break;
        }



        int bfn = atoi(buf);

        if(bfn == 0){
            printf("\n%d---womp - womped---\n", getpid());
            break;
        }

        printf("%d: %d\n", getpid(), bfn);

        int cd = bfn + rand()%21 - 10;
        snprintf(buf, 20, "%d", cd);
        if(write(fdw, buf, strlen(buf)+ 1) < 0)
            ERR("write");
    }
    if(close(fdw) < 0)
            ERR("close fdw");
        if(close(fdr) < 0)
            ERR("close fdr");
    return;
}

void parent_work(int fdr, int fdw){
    char buf[20];
    snprintf(buf, 20, "%d", 1);
    if(write(fdw, buf, strlen(buf) + 1) < 0)
        ERR("write");


    while(1){
            
            int broken = read(fdr, buf, 20);

        if(broken < 0)  ERR("read");
        if(broken == 0){
            printf("\n%d: ZERWANOPIPE RODICA \n", getpid());
            break;
        }


            
            int bfn = atoi(buf);
            if(bfn == 0){
                printf("\n%d---womp - womped---\n", getpid());
                break;
            }
            printf("parent %d: %d\n", getpid(), bfn);

            int cd = bfn + rand()%21 - 10;
            snprintf(buf, 20, "%d", cd);

            if(write(fdw, buf, strlen(buf) + 1) < 0)
                ERR("write");
        }
        if(close(fdw) < 0)
            ERR("close fdw");
        if(close(fdr) < 0)
            ERR("close fdr");
    return;
}

int main(int argc, char** argv){
    int *fds;
    fds = malloc(sizeof(int)* 6);

    pipe(&fds[0]);
    pipe(&fds[2]);
    pipe(&fds[4]);
    for(int i = 0; i < 2; i++){
        switch(fork()){
            case 0:
                for(int j = 0; j < 6; j++){
                    if(j != 2*i && j != 2*i +3){
                        if(close(fds[j]) < 0)
                            ERR("close");
                    }
                }
                child_work(fds[2*i], fds[2*i + 3]);
                exit(EXIT_SUCCESS);
            case -1:
                ERR("Fork:");
        }
    }  
    if(close(fds[0]) < 0) ERR("close");
    if(close(fds[2]) < 0) ERR("close");
    if(close(fds[3]) < 0) ERR("close");
    if(close(fds[5]) < 0) ERR("close");

    parent_work(fds[4], fds[1]);

    while(wait(NULL) > 0)
        printf("waiting\n");
    free(fds);
    
}