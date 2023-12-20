#ifndef STDLIB_SYSCALL_H
#define STDLIB_SYSCALL_H

#pragma GCC system_header

#define SYSCALL_RV_SYS_READ 63
#define SYSCALL_RV_SYS_WRITE 64
#define SYSCALL_RV_SYS_EXIT 93
#define SYSCALL_RV_SYS_BRK 214

static inline long __internal_syscall(long n, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5) {
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

#endif  // STDLIB_SYSCALL_H
