#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>


#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define BUF_S 16

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s N M\n", name);
    fprintf(stderr, "2<=N<=5 \n");
    fprintf(stderr, "5<=M<=10 \n");

    exit(EXIT_FAILURE);
}


void child_work(int fdr, int fdw, int M){
    srand(getpid());
    
    int cards[M+1];
    memset(cards, 0, sizeof(cards));
    int total_score = 0;

    while(1){
        char message[BUF_S] = {0};
        int r = read(fdr, message, BUF_S);
        if(r < 0) ERR("read");
        if(r == 0) {printf("zerwano pipe"); break;}
        
        if(strcmp(message, "new_round") == 0)
        {
            if(rand()%100 <= 20){
                printf("WYJEBAŁEM SIĘ XDDDD");
                break;
            }
            int card;
            do{
                card = rand()%M + 1;
            }while(cards[card] == 1);

            cards[card] = 1;

            char buf[BUF_S] = {0};
            snprintf(buf, BUF_S, "%d", card);

            if(write(fdw, buf, BUF_S)< 0) ERR("write");
        }
        else{
            total_score += atoi(message);
            //printf("\n--- dostalem %d punktow i mam teraz %d \n\n", atoi(message), total_score);
        }

    }
    printf("\n---%d Skonczylem z %d punktami\n", getpid(), total_score);
    if(close(fdr) < 0) ERR("close");
    if(close(fdw) < 0) ERR("close");

}
void parent_work(int* fds, int N, int M){

    char message[BUF_S] = {0};
    strcpy(message, "new_round");
    int alive[N];
    for(int i = 0; i < N; i++) alive[i] = 1;
    for(int k = 0; k < M; k++){
        printf("===NEW ROUND\n");
        for(int i = 0; i < N; i++){
            if(alive[i] == 1)
                if(write(fds[4*i + 1], message, BUF_S) < 0) ERR("write");
        }
        
        

        char buf[BUF_S] = {0};
        int scores[N];

        int max = -1;


        for(int i = 0; i < N; i ++){
            if(alive[i] == 0)
            {
                scores[i] = -1;
                continue;
            }

            int r = read(fds[4*i + 2], buf, BUF_S);
            if(r < 0) ERR("read");
            if(r == 0){
                printf("===Gracz %d umar womp womp\n", i+1);
                alive[i] = 0;
                scores[i] = -1;
                continue;
            }
            printf("===Got number %s from player %d\n", buf, i+1);
            scores[i] = atoi(buf);
            if(scores[i] > max){
                max = scores[i];
            }
        }

        int winners = 0;
        for(int i = 0; i < N; i++){
            if(scores[i] == max){
                winners++;
            }
        }

        int player_score = N / winners;

        
        for(int i = 0; i < N; i++){
            if(alive[i] == 0)
                continue;
            char buf_score[BUF_S] = {0};
            int r;
            if(scores[i] == max){
                printf("\n\n=== gracz %d dostaje %d punktow\n\n", i+1, player_score);
                snprintf(buf_score, BUF_S, "%d", player_score);
                r = write(fds[4*i + 1], buf_score, BUF_S);
                if(r < 0) ERR("write");
                if(r == 0)
                    printf("ojojj");
            }
            else{
                snprintf(buf_score, BUF_S, "%d", 0);
                r = write(fds[4*i + 1], buf_score, BUF_S);
                if(r < 0) ERR("write");
                if(r == 0)
                    printf("hahahaahahahah");
            }
        }

        // int player_score = N;
        // char buf_score[BUF_S] = {0};
        // if(max1 == max2 && imax1 != imax2){
        //     player_score /= 2;    
        //     snprintf(buf_score, BUF_S, "%d", player_score);
        //     if(write(fds[1 + 4*imax1], buf_score, BUF_S) < 0)
        //         ERR("write");
        //     if(write(fds[1 + 4*imax2], buf_score, BUF_S) < 0)
        //         ERR("write");

        //     for(int h = 0; h < N; h++){
        //         if(h != imax1 && h != imax2){
        //             snprintf(buf_score, BUF_S, "%d", 0);
        //             if(write(fds[1 + 4*h], buf_score, BUF_S) < 0)
        //                 ERR("write");
        //         }
        //     }
        // }
        // else{
        //     snprintf(buf_score, BUF_S, "%d", player_score);
        //     if(write(fds[1 + 4*imax1], buf_score, BUF_S) < 0)
        //         ERR("write");

        //     for(int h = 0; h < N; h++){
        //         if(h != imax1){
        //             snprintf(buf_score, BUF_S, "%d", 0);
        //             if(write(fds[1 + 4*h], buf_score, BUF_S) < 0)
        //                 ERR("write");
        //     }
        // }
        
    }
    for(int i = 0; i < 4*N; i++){
        if(i%4 == 1 || i%4 == 2)
            if(close(fds[i]) < 0) ERR("close");
    }
}


int main(int argc, char** argv){
    if(argc != 3){
        usage(argv[0]);
    }
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    if(N < 2 || N > 5 || M < 5 || M > 10){
        usage(argv[0]);
    }

    int fds[4*N];
    for(int i = 0; i < 4*N; i+=2){
        if(pipe(&fds[i]) < 0)
            ERR("pipe");
    }

    for(int i = 0; i < N; i++){
        switch(fork()){
            case 0:
                for(int j = 0; j < 4*N; j++){
                    if(j != 4*i && j != 4*i + 3){
                        if(close(fds[j]) < 0) ERR("close");
                    }
                }
                child_work(fds[4*i], fds[4*i + 3], M);
                exit(EXIT_SUCCESS);
            case -1:
                ERR("fork");
        }
    }
    for(int j = 0; j < 4*N; j++){
        if(j%4 != 1 && j%4 != 2){
            if(close(fds[j]) < 0) ERR("close");
        }
    }
    parent_work(fds, N, M);

    while(wait(NULL) > 0);
}