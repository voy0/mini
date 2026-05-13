#include "w7-common.h"
#include <netinet/in.h>
#include <stdint.h>

#define BACKLOG 5
#define MAX_EVENTS 16

volatile sig_atomic_t do_work = 1;
void sigint_handler(int sig) { do_work = 0; }

void usage(char *name) { fprintf(stderr, "USAGE: %s socket port\n", name); }

int calc(char *data) {
  int res = 0;
  for (int i = 0; i < strlen(data); i++) {
    if (data[i] >= '0' && data[i] <= '9') {
      res += (data[i] - '0');
    }
  }
  return res;
}
void doServer(int tcp_listen_socket) {

  char data[32];
  ssize_t size;
  int max = 0;

  int epoll_descriptor;
  if ((epoll_descriptor = epoll_create1(0)) < 0) {
    ERR("epoll_create:");
  }
  struct epoll_event event, events[MAX_EVENTS];
  event.events = EPOLLIN;
  event.data.fd = tcp_listen_socket;
  if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, tcp_listen_socket, &event) ==
      -1) {
    perror("epoll_ctl: listen_sock");
    exit(EXIT_FAILURE);
  }

  int nfds;
  sigset_t mask, oldmask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_BLOCK, &mask, &oldmask);
  while (do_work) {
    if ((nfds = epoll_pwait(epoll_descriptor, events, MAX_EVENTS, -1,
                            &oldmask)) > 0) {
      for (int n = 0; n < nfds; n++) {
        int client_socket = add_new_client(tcp_listen_socket);
        printf("[S]: Polaczono klienta\n");

        if ((size = bulk_read(client_socket, data, sizeof(data))) < 0)
          ERR("read:");
        printf("[S]: Odebrano dane klienta\n");
        printf("[S<-C]: %s\n", data);

        if (size == (int)sizeof(data)) {
          int16_t res = calc(data);
          if (res > max)
            max = res;
            int16_t nres = htons(res);
          if (bulk_write(client_socket, (char *)&nres, sizeof(int16_t)) < 0 &&
              errno != EPIPE)
            ERR("write:");
          printf("[S]: Wyslano dane klienta\n");
          printf("[S->C]: %d\n", res);
        }
        if (TEMP_FAILURE_RETRY(close(client_socket)) < 0)
          ERR("close");
      }
    }
    else
        {
            if (errno == EINTR)
                continue;
            ERR("epoll_pwait");
        }
  }
  if (TEMP_FAILURE_RETRY(close(epoll_descriptor)) < 0)
        ERR("close");
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
  printf("Treminating...\n");
  printf("Highest result: %d\n", max);
}

int main(int argc, char **argv) {
  char *name = argv[0];
  if (argc != 2) {
    usage(name);
    exit(EXIT_FAILURE);
  }
  if (sethandler(SIG_IGN, SIGPIPE))
    ERR("setting SIGPIPE");
  if (sethandler(sigint_handler, SIGINT))
    ERR("Setting SIGINT");
  int tcp_listen_socket;
  tcp_listen_socket = bind_tcp_socket(atoi(argv[1]), BACKLOG);

  //   int new_flags = fcntl(tcp_listen_socket, F_GETFL) | O_NONBLOCK;
  //   fcntl(tcp_listen_socket, F_SETFL, new_flags);
  doServer(tcp_listen_socket);

  if (TEMP_FAILURE_RETRY(close(tcp_listen_socket)) < 0)
    ERR("close");
  fprintf(stderr, "Server has terminated.\n");
  return EXIT_SUCCESS;
}