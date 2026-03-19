#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_PATH 101

void scan_dir()
{
    DIR *dir;
    struct dirent *dp;
    struct stat filestat;
    int files = 0, links = 0, dirs = 0, others = 0;
    if((dir = opendir(".")) == NULL){
        ERR("opendir");
    }
    do
    {
        errno = 0;
        if((dp = readdir(dir)) != NULL)
        {
            if(lstat(dp->d_name, &filestat))
                ERR("lstat");
            if(S_ISREG(filestat.st_mode))
                files++;
            else if(S_ISDIR(filestat.st_mode))
                dirs++;
            else if(S_ISLNK(filestat.st_mode))
                links++;
            else
                others++;
        }

    }while(dp !=0);
    if(errno != 0)
        ERR("readdir");
    if(closedir(dir))
        ERR("closedir");
    printf("Files: %d, Dirs: %d, Links: %d, Others: %d\n", files, dirs, links, others);
    
}

int main(int argc, char** argv)
{
    char path[MAX_PATH];
    if(getcwd(path, MAX_PATH) == NULL)
    {
        ERR("getcwd");
    }
    for(size_t i = 1; i < argc; i++)
    {
        if(chdir(argv[i]))
            ERR("chdir");
        printf("%s:\n", argv[i]);
        scan_dir();
        if(chdir(path))
            ERR("chdir");
    }
    return EXIT_SUCCESS;
}