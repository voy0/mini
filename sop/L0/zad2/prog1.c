#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXL 20

#define ERR(source) (perror(source),\
fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
exit(EXIT_FAILURE))

int main(int argc, char** argv){
    char name[MAXL+2];
    scanf("%21s", name);
    if(strlen(name)>MAXL){
        ERR("NAME TOO LONG");
    }
    printf("Hello %s\n", name);
    return EXIT_SUCCESS;    
}