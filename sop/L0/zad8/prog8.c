#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXL 20

#define ERR(source) (perror(source),\
fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
exit(EXIT_FAILURE))

int main(int argc, char** argv){
    int x;
    char* env = getenv("TIMES");
    if(env)
        x = atoi(env);
    else
        putenv("TIMES=10");
    printf("%d", x);
    char name[MAXL+2];
    while(fgets(name, MAXL+2, stdin) !=NULL)
        for(int i = 0; i < x; ++i)
            printf("Hello %s", name); 
    
    if(putenv("RESULT=Done") != 0){
        fprintf(stderr, "putenv failed");
        return EXIT_FAILURE;
    }

    printf("%s\n", getenv("RESULT"));
    if(system("env|grep RESULT") != 0)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;    
}