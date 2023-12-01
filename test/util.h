#pragma GCC system_header

#define NULL (void*)0

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_copy __builtin_va_copy
#define va_arg __builtin_va_arg


#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define EOF (-1)

#define SYSCALL_RV_SYS_READ  63
#define SYSCALL_RV_SYS_WRITE 64
#define SYSCALL_RV_SYS_EXIT  93


static inline long __internal_syscall(
    long n, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5) {
    register long a0 asm("a0") = _a0;
    register long a1 asm("a1") = _a1;
    register long a2 asm("a2") = _a2;
    register long a3 asm("a3") = _a3;
    register long a4 asm("a4") = _a4;
    register long a5 asm("a5") = _a5;

    register long syscall_id asm("a7") = n;

    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(syscall_id));

    return a0;
}

int isdigit(int arg) {
    return (arg >= '0' && arg <= '9');
}

char* itoa(int value, char *str, int base) {
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
        if(str[i] == '%') {
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
                    char *currC = (char*)va_arg(vl, char*);
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
        }
        else {
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


int scanf(char * str, ...) {
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
        if (str[i] == '%')  {
            i++;
            switch (str[i]) {
                case 'c': {
                    *(char*)va_arg(vl, char*) = buf[retVal];
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
                    *(int*)va_arg(vl, int*) = atoi(tmp);
                    break;
                }
                case 's': {
                    char *outStr = (char*)va_arg(vl, char*);
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
        }
        else {
            retVal++;
        }
        i++;
    }
    va_end(vl);
    return retVal;
}


void exit(int status) {
    __internal_syscall(SYSCALL_RV_SYS_EXIT, status & 0xFF, 0, 0, 0, 0, 0);
}
