#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>

#define ERR(source) (perror(source),\
fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
exit(EXIT_FAILURE))

extern char** environ;

void usage(char* pname){
    fprintf(stderr, "USAGE:%s [VARN_NAME VARN_VALUE] ... \n",
    pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv){
    //sprawdzenie czy wystepuje poprawna ilosc argumentow
    if(argc % 2 == 0) //argv[0]
    {
        usage(argv[0]);
    }

    for(int i = argc - 1; i > 0; i -= 2)
    {
        //if overwrite is unsuccessful
        if (setenv(argv[i-1], argv[i], 1))
        {
            if(EINVAL == errno)
                ERR("setenv - variable name contains '='");
            ERR("setenv");
        }
    }
    int index = 0;
    //wypisanie zmiennych srodowiskowych
    while(environ[index]){
        printf("%s\n", environ[index++]);
    }
    return EXIT_SUCCESS;
}