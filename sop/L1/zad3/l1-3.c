#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAXFD 20

int files = 0, links = 0, dirs = 0, others = 0;

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
    switch (type)
    {
        case FTW_DNR:
        case FTW_D:
            dirs++;
            break;
        case FTW_F:
            files++;
            break;
        case FTW_SL:
            links++;
            break;
        default:
            others++;
    }
    return 0;
}

int main(int argc, char** argv)
{
    for(int i = 1; i < argc; i++)
    {
        if (nftw(argv[i], walk, MAXFD, FTW_PHYS) == 0)
            printf("%s:\nFiles: %d, Dirs: %d, Links: %d, Others: %d\n", argv[i], files, dirs, links, others);
        else
            printf("%s: access denied\n", argv[i]);
        files = links = dirs = others = 0;
    }
    return EXIT_SUCCESS;
}