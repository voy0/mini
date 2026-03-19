#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if(-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}

void sig_handler(int sig) { sig_count++; }

void sigchld_handler(int sig)
{
    pid_t pid;
    for (;;)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if (pid == 0)
            return;
        if (pid <= 0)
        {
            if (errno == ECHILD)
                return;
            ERR("waitpid");
        }
    }
}


ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void child_write_file(int s, int n, char* pid)
{
    //tworzenie bloku
    char* buf = malloc(s);
    if (!buf) ERR("malloc");
    memset(buf, n, s);
    int out;
    char name[28];
    sprintf(name, "%s.txt", pid);
    
    if((out = TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777))) < 0)
        ERR("open");
    if (bulk_write(out, buf, s) < 0)
        ERR("read");
    if (TEMP_FAILURE_RETRY(close(out)))
        ERR("close");
    free(buf);
}

void child_work(int n)
{
    srand(time(NULL) * getpid());
    int s = rand()%(100-10 + 1) + 10;
    s *= 1024;
    char pid[24];
    sprintf(pid, "%d", getpid());
    printf("PROCESS with pid %s, n %d and size %d\n", pid, n, s);
    child_write_file(s, n, pid);
}

void parent_work()
{
    struct timespec t = {0, 10000000};
    for(int i = 0; i < 100; i++)
    {
        nanosleep(&t, NULL);
        if(kill(0, SIGUSR1))
            ERR("kill");
    }
}

void create_children(int n, char** argv)
{
    pid_t pid;
    for(int i=1; i < n; i++)
    {
        if((pid=fork()) < 0)
            ERR("Fork:");
        if(pid==0)
        {
            sethandler(sig_handler, SIGUSR1);
            child_work(atoi(argv[i]));
            exit(EXIT_SUCCESS);
        }
        else
        {
            parent_work();        
        }
    }
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n [0-9]\n", name);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    if(argc < 2)
        usage(argv[0]);
    for(int i = 1; i < argc; i++)
    {
        if(atoi(argv[i]) < 0 || atoi(argv[i]) > 9)
            usage(argv[0]);
    }
    sethandler(sigchld_handler, SIGCHLD);
    sethandler(sig_handler, SIGUSR1);
    create_children(argc, argv);
    
    return EXIT_SUCCESS;
}