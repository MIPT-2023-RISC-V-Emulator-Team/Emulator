#include "stdlib/stdio.h"

int main() {
    puts("Enter a message:");

    char c;
    while ((c = getchar()) != EOF && c != '\n') {
        putchar(c);
    }
    putchar('\n');

    puts("Enter another message:");
    char str[1024];
    gets(str);
    puts(str);

    puts("Enter yet another message:");
    scanf("%s", str);
    puts(str);
    return 0;
}
