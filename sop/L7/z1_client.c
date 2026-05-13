#include "w7-common.h"
#include <netinet/in.h>

void usage(char *name) { fprintf(stderr, "USAGE: %s socket port\n", name); }

int main(int argc, char **argv)
{
    int fd;
    if (argc != 3)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    fd = connect_tcp_socket(argv[1], argv[2]);
    sleep(1);
    int32_t data = getpid();
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", getpid());

    if (bulk_write(fd, buf, sizeof(buf)) < 0)
        ERR("write:");

    int16_t nres;
    if (bulk_read(fd, (char*)&nres, sizeof(int16_t)) < (int)sizeof(int16_t))
        ERR("read:");
    int16_t res = ntohs(nres);
        printf("[PID=%d]: %d\n", getpid(), res);
    
    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
    return EXIT_SUCCESS;
}