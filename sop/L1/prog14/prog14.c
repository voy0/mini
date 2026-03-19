#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define FILE_BUF_LEN 256

void usage(const char *const pname)
{
    fprintf(stderr, "USAGE:%s path_from path_to\n", pname);
    exit(EXIT_FAILURE);
}

ssize_t bulk_read(int fd, char *buf, size_t count) 
//definiujemy ssize zeby mozna bylo zwrocic blad przez return -1
{
    ssize_t c; // ilosc wczytanych bajtow
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if(c < 0)
            return c;
        if(c == 0)
            return len; //EOF
        buf += c;
        len += c;
        count -= c;
    } while(count > 0);
    return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c; // ilosc wczytanych bajtow
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if(c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while(count > 0);
    return len;
}

int main(int argc, char **argv)
{
    if(argc != 3)
        usage(argv[0]);

    const char const* path_from = argv[1];
    const char const* path_to = argv[2];
    
    const int fd_from  = open(path_from, O_RDONLY);
    if(fd_from == -1)
        ERR("open");
    
    const int fd_to  = open(path_to, O_WRONLY | O_CREAT, 0777);
    if(fd_to == -1)
        ERR("open");

    char buf[FILE_BUF_LEN];
    
    while(true)
    {
        const ssize_t read_size = bulk_read(fd_from, buf, FILE_BUF_LEN);
        if(read_size == -1)
            ERR("bulk_read");

        if(read_size == 0)
            break;

        if(bulk_write(fd_to, buf, read_size) == -1)
            ERR("bulk_write");
    }

    if(close(fd_from) == -1)
        ERR("close");

    if(close(fd_to) == -1)
        ERR("close");

    return EXIT_SUCCESS;
}