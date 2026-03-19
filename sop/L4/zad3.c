#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))
#define S_BUF 32

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "3<=n<=20 \n");

    exit(EXIT_FAILURE);
}

volatile sig_atomic_t the_end = 0;

int sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void sigalrm_handler(int sig){
    the_end = 1;
}


void student_work(int tr, int tw, int idx){

    // ODPOWIEDZ NA SPRAWDZENIE OBECNOSCI

    char message[S_BUF] = {0};
    int r = read(tr, message, S_BUF); 

    if(r < 0) ERR("read");
    else if(r == 0) printf("Student %d: skończyłem prace", getpid());
    else{
        memset(message, 0, S_BUF);
        snprintf(message, S_BUF, "Student %d: JESTEM", getpid());
        printf("%s\n", message);
    }

    if(write(tw, message, S_BUF) < 0) ERR("write");

    // PRACA NAD ZADANIEM
    
    srand(getpid());
    int k = 3 + rand()%(9 - 3 + 1);
    int stage = 0;

    while(1){
        if(the_end == 1){
            printf("Student %d: Oh no, I haven't finished stage %d. I need more time\n", getpid(), stage);
            break;
        }

        int t = 100 + rand()%(501-100);
        usleep(t * 1000);

        int q = 1 + rand() % (20);

        int attempt_result = k + q;

        memset(message, 0, S_BUF);
        snprintf(message, S_BUF, "%d %d %d %d", idx, getpid(), stage, attempt_result);

        int w = write(tw, message, S_BUF);
        
        if(w <= 0){
            printf("Student %d: Oh no, I haven't finished stage %d. I need more time\n", getpid(), stage);
            break;
        }


        memset(message, 0, S_BUF);
        int r = read(tr, message, S_BUF);
        if(r <= 0) {
            printf("Student %d: Oh no, I haven't finished stage %d. I need more time\n", getpid(), stage);
            break;
        }

        int passed = 0;
        sscanf(message, "%d", &passed);

        if(passed == 1){
            stage++;
        }
        if(stage == 4){
            printf("Student %d: I NAILED IT!\n", getpid());
            break;
        }
    }

    if(close(tr) < 0) ERR("close");
    if(close(tw) < 0) ERR("close");

}
void teacher_work(int sr, int* sw, int* pids, int n){
    char message[S_BUF] = {0};
    int stages[4] = {3, 6,7, 5};
    
    // SPRAWDZANIE OBECNOSCI
    for(int i = 0; i < n; i++){
        snprintf(message, S_BUF, "Teacher: Is %d here?\n", pids[i]);
        printf("%s\n", message);
        if(write(sw[i*2 + 1], message, S_BUF) < 0) ERR("write");
    }

    // OTRZYMYWANIE ODPOWIEDZI OBECNOSCI
    for(int i = 0; i < n; i++){
        int r = read(sr, message, S_BUF);
        if(r < 0) ERR("read");
        else if(r == 0) printf("Nie ma jebanego %d\n", pids[i]);
        else{
            printf("Od Studenta (%s)\n", message);
        }
    }
    alarm(2);
    //sleep(3);

    // SPRAWDZANIE ETAPOW
    int students_finished = 0;
    int r_pid, r_stage, r_score, r_id;
    while(1){

        if(the_end == 1){
            printf("Teacher: END OF TIME!\n");
            break;
        }

        int r = read(sr, message, S_BUF);
        if(r < 0) {
            if(errno == EINTR && the_end == 1){
                printf("Teacher: END OF TIME2!\n");
                break;
            }
            else if(errno != EINTR){
                ERR("read");
            }
        }
        else if(r == 0)break;
        else{
            sscanf(message, "%d %d %d %d", &r_id, &r_pid, &r_stage, &r_score);
            int d = stages[r_stage] + rand()%(20) + 1;
            memset(message, 0, S_BUF);
            if(r_score >= d){
                snprintf(message, S_BUF, "%d", 1);
                printf("Teacher: Student %d finished stage %d\n", r_pid, r_stage+1);
                if(r_stage == 3){
                    students_finished++;
                }
            }
            else{
                snprintf(message, S_BUF, "%d", 0);
                printf("Teacher: Student %d failed stage %d\n", r_pid, r_stage+1);
            }
            if(write(sw[r_id*2 + 1], message, S_BUF) < 0 ) ERR("write");
        }
        if(students_finished == n) {
            printf("\nTeacher: IT'S FINALLY OVER!\n");
            break;
        }
    }
    
    if(close(sr) < 0) ERR("close");
    
    for(int i = 0; i < n; i++){
        if(close(sw[2*i + 1]) < 0) ERR("close");
    }
}


int main(int argc, char** argv){
    signal(SIGPIPE, SIG_IGN);
    if(argc != 2) usage(argv[0]);
    
    int n = atoi(argv[1]);
    if(n < 3 || n > 20) usage(argv[0]);
    
    // TWORZENIE PIP
    int s2t[2];
    if(pipe(s2t) < 0) ERR("pipe");

    int t2s[2*n];
    for(int i = 0; i < n; i++){
        if(pipe(&t2s[i*2]) < 0) ERR("pipe");
    }

    int pids[n];
    for(int i = 0; i < n; i++){
        int pid = fork();
        switch(pid){
            case 0:
                printf("%d\n", getpid());
                // ZAMYKANIE NIEPOTRZEBNYCH FDS DLA STUDENTOW
                for(int j = 0; j < 2*n; j++){
                    if(j != 2*i){
                        if(close(t2s[j]) < 0) ERR("close");
                    }
                }
                if(close(s2t[0]) < 0) ERR("close");
                

                // ODPALANIE STUDENT
                student_work(t2s[2*i], s2t[1], i);

                exit(EXIT_SUCCESS);
                break;
            case -1:
                ERR("fork");
                break;
            default:
                pids[i] = pid;
                break;
        }
    }

    // ZAMYKANIE NIEPOTRZEBNCYH FDS DLA TEACHERA
    for(int i = 0; i < 2*n; i += 2){
        if(close(t2s[i]) < 0) ERR("close");
    }
    if(close(s2t[1]) < 0) ERR("close");

    sethandler(sigalrm_handler, SIGALRM);
    // ODPALANIE TEACHER
    teacher_work(s2t[0], t2s, pids, n);

    while(wait(NULL) > 0);
}