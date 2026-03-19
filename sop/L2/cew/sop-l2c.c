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
#include <signal.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

void sethandler(void (*f)(int), int sigNo) {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (sigaction(sigNo, &act, NULL) < 0)
        ERR("sigaction");
}

void sig_handler(int sig) {
    last_signal = sig;
}

void sleep_ms(int ms) {
    struct timespec req, rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    while (nanosleep(&req, &rem) < 0) {
        if (errno == EINTR) {
            req = rem;  // Kontynuuj spanie pozostały czas
        } else {
            ERR("nanosleep");
        }
    }
}

void transform_and_write(const char *fragment, size_t size, const char *output_file) {
    int fd = TEMP_FAILURE_RETRY(open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644));
    if (fd < 0)
        ERR("open output file");

    for (size_t i = 0; i < size; i++) {
        char c = fragment[i];
        if ((i % 2 == 1) && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            if (c >= 'a' && c <= 'z')
                c -= 32; // Mała -> Wielka
            else
                c += 32; // Wielka -> Mała
        }
        if (bulk_write(fd, &c, 1) < 0)
            ERR("write");
        sleep_ms(250);  // Śpij 0.25 sekundy
    }

    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
}

void child_work(const char *fragment, size_t size, const char *output_file) {
    sigset_t mask;
    sigemptyset(&mask);

    while (last_signal != SIGUSR1)
        sigsuspend(&mask);  // Oczekiwanie na SIGUSR1

    printf("Child PID: %d starts processing\n", getpid());
    transform_and_write(fragment, size, output_file);
    exit(EXIT_SUCCESS);
}

void parent_work(const char *f, int n) {
    int fd = TEMP_FAILURE_RETRY(open(f, O_RDONLY));
    if (fd < 0)
        ERR("open");

    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0)
        ERR("fstat");

    size_t file_size = file_stat.st_size;
    char *buf = malloc(file_size);
    if (!buf)
        ERR("malloc");

    if (bulk_read(fd, buf, file_size) != file_size)
        ERR("read");
    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");

    size_t chunk_size = file_size / n;
    size_t last_chunk_size = file_size % n == 0 ? chunk_size : chunk_size + file_size % n;

    pid_t *children = malloc(n * sizeof(pid_t));
    if (!children)
        ERR("malloc");

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0)
            ERR("fork");
        if (pid == 0) {  // Proces potomny
            sethandler(sig_handler, SIGUSR1);
            char output_file[256];
            snprintf(output_file, sizeof(output_file), "%s-%d", f, i + 1);

            size_t current_chunk_size = (i == n - 1) ? last_chunk_size : chunk_size;
            child_work(buf + i * chunk_size, current_chunk_size, output_file);
        } else {
            children[i] = pid;
        }
    }

    free(buf);

    // Wysłanie SIGUSR1 do wszystkich dzieci
    for (int i = 0; i < n; i++) {
        if (kill(children[i], SIGUSR1) < 0)
            ERR("kill");
    }

    // Czekanie na zakończenie wszystkich dzieci
    for (int i = 0; i < n; i++) {
        if (waitpid(children[i], NULL, 0) < 0)
            ERR("waitpid");
    }

    free(children);
}

void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s file n\n", prog_name);
    fprintf(stderr, "\tfile - file to process\n");
    fprintf(stderr, "\tn - number of child processes (0 < n < 10)\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3)
        usage(argv[0]);

    int n = atoi(argv[2]);
    if (n <= 0 || n >= 10)
        usage(argv[0]);

    const char *f = argv[1];

    sethandler(sig_handler, SIGINT);  // Obsługa Ctrl-C
    sethandler(sigchld_handler, SIGCHLD);

    parent_work(f, n);

    return EXIT_SUCCESS;
}
