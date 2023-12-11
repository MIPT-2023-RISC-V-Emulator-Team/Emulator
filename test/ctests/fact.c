#include "stdlib/stdio.h"

int main() {
    long fact = 1;
    for (long i = 1; i < 12; i++) {
        fact *= i;
        printf("%d! = %d\n", i, fact);
    }

    return 0;
}
