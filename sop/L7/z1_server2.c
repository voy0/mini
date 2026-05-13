#include "w7-common.h"
#include <stdlib.h>

#define BACKLOG 5
#define MAX_EVENTS 16

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
  int epoll_descriptor;
  if ((epoll_descriptor = epoll_create1(0)) < 0) {
    ERR("epoll_create");
  }
  struct epoll_event event, events[MAX_EVENTS];
  event.events = EPOLLIN;
  event.data.fd = tcp_listen_socket;
  if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, tcp_listen_socket, &event) ==
      -1) {
    perror("epoll_ctl: listen_sock");
    exit(EXIT_FAILURE);
  }

  char data[32];
  ssize_t size;
  int nfds;
  while (1) {
    if ((nfds = epoll_wait(epoll_descriptor, events, MAX_EVENTS, -1)) > 0) {
      for (int n = 0; n < nfds; n++) {
        int client_socket = add_new_client(events[n].data.fd);
        printf("[S]: Polaczono klienta\n");
        if ((size = bulk_read(client_socket, data, sizeof(data))) < 0)
          ERR("read:");
        printf("[S]: Odebrano dane klienta\n");
        printf("[S<-C]: %s\n", data);
        if (size == (int)sizeof(data)) {
          int16_t res = calc(data);

          if (bulk_write(client_socket, (char *)&res, sizeof(int16_t)) < 0 &&
              errno != EPIPE)
            ERR("write:");
          printf("[S]: Wyslano dane klienta\n");
          printf("[S<-C]: %d\n", res);
        }
        if (TEMP_FAILURE_RETRY(close(client_socket)) < 0)
          ERR("close");
      }
    } else {
      if (errno == EINTR)
        continue;
      ERR("epoll_wait");
    }
  }
}

int main(int argc, char **argv) {
  char *name = argv[0];
  if (argc != 2) {
    usage(name);
    exit(EXIT_FAILURE);
  }

  int tcp_listen_socket;
  tcp_listen_socket = bind_tcp_socket(atoi(argv[1]), BACKLOG);

  int new_flags = fcntl(tcp_listen_socket, F_GETFL) | O_NONBLOCK;
  fcntl(tcp_listen_socket, F_SETFL, new_flags);
  doServer(tcp_listen_socket);

  if (TEMP_FAILURE_RETRY(close(tcp_listen_socket)) < 0)
    ERR("close");
  fprintf(stderr, "Server has terminated.\n");
  return EXIT_SUCCESS;
}