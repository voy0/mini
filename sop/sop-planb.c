#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_GROUPS 20
#define MAX_PATH 256

int groups_count[MAX_GROUPS + 1];

void usage(char* name)
{
    fprintf(stderr, "USAGE: %s path\n", name);
    exit(EXIT_FAILURE);
}

int is_group(char* name)
{
    if (strncmp(name, "grupa", 5) == 0)
    {
        int l = strlen(name);
        char num[3];
        if (l == 7)
        {
            num[0] = name[6];
        }
        else if (l == 8)
        {
            num[0] = name[6];
            num[1] = name[7];
        }
        return atoi(num);
    }
    return 0;
}

void get_groups_count(const char* dirpath, int counts[MAX_GROUPS + 1])
{
    FILE* f;
    DIR* dirname;
    struct dirent* file;
    struct stat filestat;
    int num;
    char num2[10];

    if (chdir(dirpath) == -1)
        ERR("chdir");
    if ((dirname = opendir(".")) == NULL)
        ERR("opendir");

    do
    {
        if ((file = readdir(dirname)) != NULL)
        {
            if (lstat(file->d_name, &filestat))
                ERR("lstat");
            if ((num = is_group(file->d_name)) != 0)
            {
                f = fopen(dirpath, "r");
                fgets(num2, 10, f);
                printf("Grupa %d zawiera %s", num, num2);
            }
        }
    } while (file != NULL);
    if (errno != 0)
        ERR("readdir");
    if (closedir(dirname) == -1)
        ERR("closedir");
}

void process_file(const char* filepath, int group_count) { printf("Przetworzenie pliku z ocenami (TODO)\n"); }

void batch_process(const char* dirpath) { printf("Przetworzenie ocen w drzewie katalogow (TODO)\n"); }

int main(int argc, char** argv)
{
    struct stat filestat;
    char word[10];
    int group_count;
    if (argc != 2)
        usage(argv[0]);

    if (scanf("%9s", word) == EOF)
        ERR("scanf");

    if (lstat(argv[1], &filestat))
        ERR("lstat");
    if (strcmp(word, "groups") == 0)
    {
        if (S_ISDIR(filestat.st_mode))
            get_groups_count(argv[1], groups_count);
        else
            ERR("S_ISDIR");
    }
    else if (strcmp(word, "process") == 0)
    {
        if (S_ISREG(filestat.st_mode))
        {
            printf("Podaj liczbe studentow: \n");
            char c[100];
            scanf("%s", c);
            group_count = atoi(c);
            process_file(argv[1], group_count);
        }
        else
            ERR("S_ISREG");
    }
    else if (strcmp(word, "batch") == 0)
    {
        if (S_ISDIR(filestat.st_mode))
            batch_process(argv[1]);
        else
            ERR("S_ISDIR");
    }
    else
        usage(argv[0]);

    return EXIT_SUCCESS;
}
