#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> //opendir, readdir, closedir
#include <unistd.h> //getopt
#include <errno.h> //erno
#include <sys/stat.h> //lstat
#include <string.h> //strstr
#include <stdbool.h>
#include <ftw.h>

#define MAX 256
#define MAX_DIRS 100

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

char* ext;

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -p PATH\n", pname);
    exit(EXIT_FAILURE);
}

bool has_extension(const char *filename, const char *extension) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return false;
    }
    return strcmp(dot + 1, extension) == 0;
}

void write_dir(const char* path)
{
    DIR* dir;
    struct dirent* file;
    struct stat filestat;

    if(chdir(path) == -1)
        ERR("chdir");
    if((dir = opendir(".")) == NULL)
        ERR("opendir");


    printf("path: %s\n", path);

    do
    {
        if((file = readdir(dir)) != NULL)
        {
            if(lstat(file->d_name, &filestat))
                ERR("lstat");

            if(has_extension(file->d_name, ext))
            {
                printf("%s %ld\n", file->d_name, filestat.st_size);
            }
        }
    }while(file != NULL);
    if(errno != 0)
        ERR("readdir");
    if(closedir(dir) == -1)
        ERR("closedir");
}

int rdir(const char* name, const struct stat* s, int type, struct FTW* f)
{
    write_dir(name);
}

int main(int argc, char** argv)
{
    char c;
    char dirs[MAX_DIRS][MAX];
    char path[MAX];
    int i_dirs = 0, depth = 1;
    int e_flag = 0;
    while((c = getopt(argc, argv, "p:e:d:")) != -1)
    {
        switch(c)
        {
            case 'p':
                strcpy(dirs[i_dirs], optarg);
                i_dirs++;
                break;
            case 'e':
                if(!e_flag)
                {
                    strcpy(ext, optarg);
                    e_flag = 1;
                }
                break;
            case 'd':
                depth = atoi(optarg);
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }
    if(getcwd(path, MAX) == NULL)
        ERR("getcwd");
    for(int i = 0; i < i_dirs; i++)
    {
        if(depth == 1)
            write_dir(dirs[i]);
        else
            nftw(dirs[i], rdir, depth, FTW_PHYS);
        if(chdir(path) == -1)
            ERR("chdir");
    }
    return EXIT_SUCCESS;
}