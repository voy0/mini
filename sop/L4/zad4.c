#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define S_BUF 64
#define N_RLT 36

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "N >= 1 \n");
    fprintf(stderr, "M >= 100 \n");

    exit(EXIT_FAILURE);
}

void player_work(int idx, int dr, int dw, int m){
    int pid = getpid();
    srand(pid);
    int money = m;
    
    printf("%d: I have %d and I'm going to play roulette\n", pid, m);

    while(1){
        if(money == 0){
            printf("%d: I'M BROKE!\n", pid);
            break;
        }
        int bet_amount = rand()%money + 1;
        int bet_n = rand()%(N_RLT + 1);

        money -= bet_amount;

        char message[S_BUF];
        snprintf(message, S_BUF, "%d %d %d %d", idx, pid, bet_n, bet_amount);

        if(write(dw, message, S_BUF) < 0) ERR("write");

        memset(message, 0, S_BUF);
        if(read(dr, message, S_BUF) < 0) ERR("read");
        int won_amount = 0;
        sscanf(message, "%d", &won_amount);

        money += won_amount;
        printf("%d: I have won %d and my balance is %d\n", pid, won_amount, money);

        int exit = rand()%100;
        if(exit < 10){
            printf("%d: I saved %d and LEFT THIS SCAM!\n", pid, money);
            break;
        }
    }
    if(close(dr) < 0) ERR("close");
    if(close(dw) < 0) ERR("close");
}

void dealer_work(int* prw, int n){

    srand(getpid());
  
    while(1){
        int who_won_what[n];

        memset(who_won_what, 0, n*sizeof(int));
        char message[S_BUF] = {0};

        int idx, pid, bet_n, bet_amount;
        int closed_pipes = 0;
        

        int lucky_number = rand()%(N_RLT + 1);
        for(int i = 0; i < n; i++){
            int r = read(prw[i*4], message, S_BUF);
            if(r <= 0) {
                closed_pipes++;
               continue;
            }
            sscanf(message, "%d %d %d %d", &idx, &pid, &bet_n, &bet_amount);
            who_won_what[idx] = (bet_n == lucky_number) ? bet_amount*35 : 0;
            printf("Dealer: %d placed %d on %d\n", pid, bet_amount, bet_n);
        }
        if(closed_pipes >= n){
            break;
        }
        printf("Dealer: %d is the lucky number\n", lucky_number);
        for(int i = 0; i < n; i++)
        {
            memset(message, 0, S_BUF);
            snprintf(message, S_BUF, "%d", who_won_what[i]);
            int w = write(prw[i*4 + 3], message, S_BUF);
            if(w <= 0) {
                closed_pipes++;
                continue;}
        }
        
    }
    printf("Dealer: The house always wins *smug_face*\n");

    for(int i = 0; i < n; i++){
        if(close(prw[4*i]) < 0) ERR("close");
        if(close(prw[4*i + 3]) < 0) ERR("close");
    }
}

int main(int argc, char** argv){
    signal(SIGPIPE, SIG_IGN);

    if(argc != 3)   usage(argv[0]);

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    if(N < 1) usage(argv[0]);
    if(M < 100) usage(argv[0]);

    int fds[4*N];
    for(int i = 0; i < 4*N; i += 2){
        if(pipe(&fds[i]) < 0) ERR("pipe");
    }

    for(int i = 0; i < N; i++){
        switch(fork()){
            case 0:
                for(int j = 0; j < 4*N; j++){
                    if(j != 1 + 4*i && j != 2 + 4*i){
                        if(close(fds[j]) < 0) ERR("close");
                    }
                }

                player_work(i, fds[i*4 + 2], fds[i*4 + 1], M);
                exit(EXIT_SUCCESS);
            case -1:
                ERR("fork");
        }
    }
    for(int i = 0; i < 4*N; i++){
        if(i % 4 == 1 || i % 4 == 2){
            if(close(fds[i]) < 0) ERR("close");
        }
    }
    dealer_work(fds, N);
    while(wait(NULL) > 0);
}