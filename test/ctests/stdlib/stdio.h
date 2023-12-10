#ifndef STDLIB_STDIO_H
#define STDLIB_STDIO_H

#include "stdlib.h"
#include "syscall.h"

#pragma GCC system_header

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define EOF (-1)

int getchar() {
    long fileno = STDIN_FILENO;
    char c = 0;
    long retVal = __internal_syscall(SYSCALL_RV_SYS_READ, fileno, &c, 1, 0, 0, 0);
    if (retVal > 0) {
        return c;
    }
    return EOF;
}

int putchar(char c) {
    long fileno = STDOUT_FILENO;
    long retVal = __internal_syscall(SYSCALL_RV_SYS_WRITE, fileno, &c, 1, 0, 0, 0);
    if (retVal > 0) {
        return c;
    }
    return EOF;
}

int puts(const char *str) {
    long fileno = STDOUT_FILENO;
    long ptr = (long)str;
    long length = 0;

    char *tmp = str;
    while (*tmp != '\0') {
        length++;
        tmp++;
    }

    long retVal = (__internal_syscall(SYSCALL_RV_SYS_WRITE, fileno, ptr, length, 0, 0, 0) >= 0) && (putchar('\n') >= 0);
    if (retVal > 0) {
        return length + 1;
    }
    return EOF;
}

char *gets(char *str) {
    long fileno = STDIN_FILENO;
    long ptr = (long)str;
    long maxLength = 1024;

    long retVal = __internal_syscall(SYSCALL_RV_SYS_READ, fileno, ptr, maxLength, 0, 0, 0);
    if (retVal > 0) {
        if (str[retVal - 1] == '\n') {
            str[retVal - 1] = '\0';
        }
        return str;
    }
    return NULL;
}

int printf(const char *str, ...) {
    va_list vl;
    va_start(vl, str);
    int retVal = 0;
    char tmp[32] = {};

    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '%') {
            i++;
            switch (str[i]) {
                case 'c': {
                    char currC = (char)va_arg(vl, int);
                    if (putchar(currC) == EOF) {
                        return EOF;
                    }
                    retVal++;
                    break;
                }
                case 'd': {
                    char *tmpIter = itoa(va_arg(vl, int), tmp, 10);
                    while (*tmpIter != '\0') {
                        if (putchar(*tmpIter) == EOF) {
                            return EOF;
                        }
                        retVal++;
                        tmpIter++;
                    }
                    break;
                }
                case 's': {
                    char *currC = (char *)va_arg(vl, char *);
                    while (*currC != '\0') {
                        if (putchar(*currC) == EOF) {
                            return EOF;
                        }
                        retVal++;
                        currC++;
                    }
                    break;
                }
            }
        } else {
            if (putchar(str[i]) == EOF) {
                return EOF;
            }
            retVal++;
        }
        i++;
    }
    va_end(vl);
    return retVal;
}

int scanf(char *str, ...) {
    va_list vl;
    va_start(vl, str);
    int retVal = 0;
    char tmp[32];
    char buf[1024];

    if (!gets(buf)) {
        return EOF;
    }

    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '%') {
            i++;
            switch (str[i]) {
                case 'c': {
                    *(char *)va_arg(vl, char *) = buf[retVal];
                    retVal++;
                    break;
                }
                case 'd': {
                    char c;
                    int j = 0;
                    while (isdigit(buf[retVal])) {
                        tmp[j] = c;
                        j++;
                        retVal++;
                    }
                    tmp[j] = '\0';
                    *(int *)va_arg(vl, int *) = atoi(tmp);
                    break;
                }
                case 's': {
                    char *outStr = (char *)va_arg(vl, char *);
                    int j = 0;
                    while (buf[retVal] != '\0') {
                        outStr[j] = buf[retVal];
                        retVal++;
                        j++;
                    }
                    outStr[j] = '\0';
                    break;
                }
            }
        } else {
            retVal++;
        }
        i++;
    }
    va_end(vl);
    return retVal;
}

#endif  // STDLIB_STDIO_H
