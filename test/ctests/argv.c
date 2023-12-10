#include "stdlib/stdio.h"

int main(int argc, char **argv, char **envp) {
    printf("argc = %d\n\n", argc);

    int i = 0;
    while (argv[i] != NULL) {
        printf("argv[%d] = %s\n", i, argv[i]);
        i++;
    }
    putchar('\n');

    i = 0;
    while (envp[i] != NULL) {
        printf("envp[%d] = %s\n", i, envp[i]);
        i++;
    }
    putchar('\n');
    return 0;
}
