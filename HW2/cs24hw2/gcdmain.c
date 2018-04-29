#include "gcd.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Please enter 2 arguments. You provided %d\n", argc-1);
        return -1;
    }
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    if (x <= 0 || y <= 0) {
        printf("Please enter positive arguments.\n");
        return -1;
    }
    int result;
    // The gcd calculator expects the first argument to be larger, so call
    // that function with the correct order of arguments accordingly 
    if (x >= y)
        result = gcd(x, y);
    else
        result = gcd(y, x);
    printf("The GCD of %d and %d is %d\n", x, y, result);
    return 0;
}
