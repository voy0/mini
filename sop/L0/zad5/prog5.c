#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXL 20

void usage(char* pname){
    fprintf(stderr, "USAGE:%s name times>0\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv){
    if(argc != 3){
        usage(argv[0]);
    }
    int n = atoi(argv[2]);
    if(0 == n){
        usage(argv[0]);
    }
    for(int i = 0; i<n; i++){
        printf("%s\n", argv[1]);
    }
    return EXIT_SUCCESS;
}