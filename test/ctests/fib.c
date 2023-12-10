#include "stdlib/stdio.h"

int main() {
    int a = 0, b = 1;
    for (int i = 0; i < 20; ++i) {
        printf("F_%d = %d\n", i, a);
        int tmp = b;
        b += a;
        a = tmp;
    }
    return 0;
}
