#include "fact.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("There should be only one argument given. You gave %d\n", argc - 1);
        return -1;
    }
    char *arg = argv[1];
    int x = atoi(arg);
    if (x < 0) {
        printf("The provided integer should be non-negative\n");
        return -1;
    }
    
    
    int result = fact(x); 

    printf("The factorial of %d is %d\n", x, result);
    return 0;
}
