static inline long __internal_syscall(long n, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5) {
  register long a0 asm("a0") = _a0;
  register long a1 asm("a1") = _a1;
  register long a2 asm("a2") = _a2;
  register long a3 asm("a3") = _a3;
  register long a4 asm("a4") = _a4;
  register long a5 asm("a5") = _a5;

  register long syscall_id asm("a7") = n;

  asm volatile ("ecall"
		: "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(syscall_id));

  return a0;
}


char *itos(char *s, long i) {
    char *r = s;
    long t;
    if (i < 0) {
        i = -i;
        *s++ = '-';
    }

    t = i;
    do {
        ++s;
     } while (t /= 10);
    *s = '\0';
    do {
        *--s = '0' + i % 10;
    } while (i /= 10);
    return r;
}


void printString(const char* str, const long len) {
    long syscallNo = 64;        // syscall write
    
    long outFileno = 0;         // stdout fileno
    long ptr = (long)str;       // ptr
    long length = len;

    __internal_syscall(syscallNo, outFileno, ptr, length, 0, 0, 0);
}


void printNumber(const long num) {
    long tmp = num;
    char out[24];

    char* outBegin = itos(out, tmp);
    printString(out, 24);
}




int main() {

    const char msg[] = "Hello, world!\n";
    printString(msg, sizeof(msg));
    printNumber(-228);
    return 0;
}