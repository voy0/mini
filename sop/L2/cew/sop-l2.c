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
#include <limits.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len; // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
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

volatile sig_atomic_t last_signal = 0;

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}
void sig_handler(int sig) { last_signal = sig; }

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

void child_work()
{
}

void parent_work(char *f, int n)
{

    int fd;
    if ((fd = TEMP_FAILURE_RETRY(open(f, O_RDONLY))) < 0)
        ERR("open");

    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0)
        ERR("fstat");
    size_t file_size = file_stat.st_size;

    char *buf = malloc(file_size);
    if (!buf)
        ERR("malloc");

    if ((bulk_read(fd, buf, file_size)) != file_size)
        ERR("read");

    size_t child_size = file_size / n;
    size_t last_child_size = file_size % n == 0 ? child_size : child_size + file_size % n;

    int pid;
    for (int i = 0; i < n; i++)
    {
        if ((pid = fork()) < 0)
            ERR("fork");

        if (0 == pid)
        {
            sethandler(sig_handler, SIGUSR1);
            size_t current_child_size = (i == n - 1) ? last_child_size : child_size;

            char *child_buf = malloc(current_child_size + 1);
            if (!child_buf)
                ERR("malloc");
            strncpy(child_buf, buf + i * child_size, current_child_size);
            child_buf[current_child_size] = '\0';

            sigset_t mask;
            sigemptyset(&mask);
            
            printf("[%d] Process created...\n", getpid());
            printf("[%d] BUFFOR SIZE: %ld %ld\n", getpid(), child_size, strlen(child_buf));
            
            free(child_buf);
            
            exit(EXIT_SUCCESS);
        }
    }
    free(buf);
    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
    
    
    if (kill(0, SIGUSR1))
        ERR("kill");
}

void usage(int argc, char *argv[])
{
    printf("%s n f \n", argv[0]);
    printf("\tf - file to be processed\n");
    printf("\t0 < n < 10 - number of child processes\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int n = atoi(argv[2]);
    if (argc != 3 || n <= 0 || n >= 10)
        usage(argc, argv);

    char *f = argv[1];

    sethandler(sigchld_handler, SIGCHLD);
    sethandler(SIG_IGN, SIGUSR1);
    parent_work(f, n);

    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
