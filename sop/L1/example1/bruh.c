#include <unistd.h> //getopt
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> //opendir, readdir, closedir
#include <errno.h>
#include <sys/stat.h> //lstat
#include <string.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -p PATH -o OUTPUT_FILENAME\n", pname);
    exit(EXIT_FAILURE);
}

#define MAX 256

// void dir_fwrite(char* work_dir, char* fname, char* output)
// {
//     FILE* f;
//     if((f = fopen(fname, "w")) == NULL)
//         ERR("fopen");
//     fprintf(f, "%s", output);
// }

void dir_fread(char* path, FILE* output)
{
    DIR* dir;

    struct dirent* file;
    struct stat filestat;

    if(chdir(path) == -1)
        ERR("chdir");
    if((dir = opendir(".")) == NULL)
        ERR("opendir");
    if(getcwd(path, MAX) == NULL)
        ERR("getcwd");

    fprintf(output, "SCIEZKA:\n%s\nLISTA PLIKOW:\n",path);
    //printf("SCIEZKA:\n%s\nLISTA PLIKOW:\n",path);

    do
    {
        errno = 0;
        if((file = readdir(dir)) != NULL)
        {
            if(lstat(file->d_name, &filestat))
                ERR("lstat");
            fprintf(output, "%s: %ld\n",file->d_name, filestat.st_size);
            //printf("%s: %ld\n",file->d_name, filestat.st_size);

        }
    }while(file!=NULL);
    if(errno != 0)
        ERR("readdir");
    if(closedir(dir) == -1)
        ERR("closedir");
}

int main(int argc, char **argv)
{
    char c;
    char work_dir[MAX];
    char path[MAX];
    char fname[MAX] = "o.txt";
    FILE* output = stdout;
    int flag = 0, fflag = 0, ifo = 0;
    if(getcwd(work_dir, MAX) == NULL)
        ERR("getcwd");
    if(getcwd(path, MAX) == NULL)
        ERR("getcwd");
    

    while((c = getopt(argc, argv, "p:o:")) != -1)
    {
        ifo = 0;
        fflag++;
        switch(c)
        {
            case 'p':
                strcpy(path, optarg);
                break;
            case 'o':
                ifo++;
                if(!flag)
                {
                    strcpy(fname, optarg);
                    flag++;
                }
                else
                {
                    usage(argv[0]);
                }
                break;
            case '?':
            default:
                usage(argv[0]);
        }
        if(ifo == 0)
        {
            dir_fread(path, output);
            chdir(work_dir);
        }
    }  
    if(!fflag) 
        dir_fread(path, output);

    if(flag)
    {
        if((output = fopen(fname, "w")) == NULL)
            ERR("fopen");
        if(fclose(output) == EOF)
            ERR("fclose");
    }
        //dir_fwrite(work_dir, fname, output);
    return EXIT_SUCCESS;


}