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

int abs(int x) {
    if (x >= 0) {
        return x;
    }
    return -x;
}

void exit(int status) {
    __internal_syscall(SYSCALL_RV_SYS_EXIT, status & 0xFF, 0, 0, 0, 0, 0);
}

void *sbrk(long incr) {
    static unsigned long heap_end = 0;

    if (heap_end == 0) {
        long brk = __internal_syscall(SYSCALL_RV_SYS_BRK, 0, 0, 0, 0, 0, 0);
        if (brk == -1) {
            return (void *)(-1);
        }
        heap_end = brk;
    }

    if (__internal_syscall(SYSCALL_RV_SYS_BRK, heap_end + incr, 0, 0, 0, 0, 0) != heap_end + incr) {
        return (void *)(-1);
    }

    heap_end += incr;
    return (void *)(heap_end - incr);
}

struct meta_data {
    unsigned long size;
    unsigned long free;
};

void *malloc(unsigned long size) {
    if (size == 0) {
        return NULL;
    }

    char *p = sbrk(0);

    // Store size and free flag
    unsigned long totalSize = sizeof(struct meta_data) + size;
    totalSize = totalSize + (sizeof(unsigned long) - totalSize & (sizeof(unsigned long) - 1));

    void *request = sbrk(totalSize);
    if (request == (void *)(-1)) {
        return NULL;
    } else {
        struct meta_data *meta = p;
        meta->size = size;
        meta->free = 0;

        return (void *)(p + sizeof(struct meta_data));
    }
}

void free(void *p) {
    if (p == NULL) {
        return;
    }

    struct meta_data *meta = (char *)p - sizeof(struct meta_data);
    meta->free = 1;
}

#endif  // STDLIB_STDLIB_H
