#ifndef STDLIB_STDLIB_H
#define STDLIB_STDLIB_H

#include "syscall.h"

#pragma GCC system_header

#define NULL (void *)0

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_copy __builtin_va_copy
#define va_arg __builtin_va_arg

int isdigit(int arg) {
    return (arg >= '0' && arg <= '9');
}

char *itoa(int value, char *str, int base) {
    char *r = str;
    int t;
    if (value < 0) {
        value = -value;
        *str++ = '-';
    }

    t = value;
    do {
        ++str;
    } while (t /= base);
    *str = '\0';
    do {
        *--str = '0' + value % base;
    } while (value /= base);
    return r;
}

int atoi(const char *str) {
    int retVal = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        retVal = retVal * 10 + str[i] - '0';
    }
    return retVal;
}

void exit(int status) {
    __internal_syscall(SYSCALL_RV_SYS_EXIT, status & 0xFF, 0, 0, 0, 0, 0);
}

#endif  // STDLIB_STDLIB_H
