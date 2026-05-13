#include "w7-common.h"

#define BACKLOG 5
#define MAX_EVENTS 16



void usage(char *name) { fprintf(stderr, "USAGE: %s socket port\n", name); }

int calc(char* data){
    int res = 0;
    for(int i = 0; i < strlen(data); i++){
        if(data[i] >= '0' && data[i] <='9'){
            res += (data[i] - '0');
        }
    }
    return res;
}
void doServer(int tcp_listen_socket) {

  char data[32];
  ssize_t size;

  while(1){
    int client_socket = add_new_client(tcp_listen_socket);
    printf("[S]: Polaczono klienta\n");

    if ((size = bulk_read(client_socket, data, sizeof(data))) < 0)
        ERR("read:");
    printf("[S]: Odebrano dane klienta\n");
    printf("[S<-C]: %s\n", data);
    sleep(3);
    if (size == (int)sizeof(data)) {
        int16_t res = calc(data);

        if (bulk_write(client_socket, (char*)&res, sizeof(int16_t)) < 0 && errno != EPIPE)
            ERR("write 1:");
            
        printf("[S]: Pierwszy strzal poszedl. Czekam na RST od klienta...\n");
        
        // Czekamy 1 sekundę. Dajemy czas systemowi operacyjnemu 
        // na odebranie pakietu RST od martwego klienta.
        sleep(1);

        // DRUGI STRZAŁ
        // System ma już w pamięci pakiet RST. Wie, że rura jest złamana.
        // Wywołanie write w tym momencie generuje SIGPIPE i zabija serwer!
        if (bulk_write(client_socket, (char*)&res, sizeof(int16_t)) < 0 && errno != EPIPE)
            ERR("write 2:"); // <-- Program nigdy nie dojdzie do tego miejsca

        printf("[S]: Wyslano dane klienta\n");
        printf("[S->C]: %d\n", res);
    }
    if (TEMP_FAILURE_RETRY(close(client_socket)) < 0)
        ERR("close");
  }
}

int main(int argc, char **argv) {
  char *name = argv[0];
  if (argc != 2) {
    usage(name);
    exit(EXIT_FAILURE);
  }
  if(sethandler(SIG_IGN, SIGPIPE)) ERR("setting SIGPIPE");
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