#include "stdlib/stdio.h"
#include "stdlib/stdlib.h"

int main() {
    int N = 0;
    scanf("%d", &N);

    unsigned long *factorials = (unsigned long *)malloc(N * sizeof(unsigned long));

    factorials[0] = 1;
    factorials[1] = 1;

    for (int i = 2; i < N; i++) {
        factorials[i] = i * factorials[i - 1];
    }

    for (int i = 0; i < N; i++) {
        printf("factorials[%d] = %d\n", i, factorials[i]);
    }
    free(factorials);

    return 0;
}