#ifndef INCLUDE_CONSTANTS_H
#define INCLUDE_CONSTANTS_H

#include <array>
#include <cstdint>
#include <string_view>

#include "generated/InstructionTypes.h"

namespace RISCV {

static constexpr uint8_t INSTRUCTION_BYTESIZE = 4;
static constexpr uint8_t XLEN = 64;

namespace memory {

static constexpr uint32_t PAGE_BYTESIZE = 1 << 12;            // 4 KiB
static constexpr uint64_t PHYS_MEMORY_BYTESIZE = 1ULL << 30;  // 1 GiB
static constexpr uint64_t VIRT_MEMORY_BYTESIZE = 1ULL << 34;  // 16 GiB
static constexpr uint64_t PHYS_PAGE_COUNT = PHYS_MEMORY_BYTESIZE / PAGE_BYTESIZE;

static constexpr uint32_t ADDRESS_PAGE_NUM_SHIFT = 12;
static constexpr uint32_t ADDRESS_PAGE_OFFSET_MASK = 0xFFF;

static constexpr uint32_t STACK_BYTESIZE = 1 << 24;  // 16 MiB
static constexpr uint64_t DEFAULT_STACK_ADDRESS = 0x3FFFFC00;

}  // namespace memory

static constexpr uint32_t CSR_COUNT = 4096;
static constexpr uint32_t CSR_SATP_INDEX = 0x180;

// Translation modes
static constexpr uint8_t PTE_LEVELS_SV39 = 3;
static constexpr uint8_t PTE_LEVELS_SV48 = 4;
static constexpr uint8_t PTE_LEVELS_SV57 = 5;
static constexpr uint8_t PTE_LEVELS_SV64 = 6;
static constexpr uint8_t PTE_SIZE = 8;

enum TranslationMode : uint64_t {
    TRANSLATION_MODE_BARE = 0,
    TRANSLATION_MODE_SV39 = 8,
    TRANSLATION_MODE_SV48 = 9,
    TRANSLATION_MODE_SV57 = 10,
    TRANSLATION_MODE_SV64 = 11
};

enum RegisterType : uint8_t {

    /*
     * Numeric names
     */

    X0 = 0,
    X1,
    X2,
    X3,
    X4,
    X5,
    X6,
    X7,
    X8,
    X9,
    X10,
    X11,
    X12,
    X13,
    X14,
    X15,
    X16,
    X17,
    X18,
    X19,
    X20,
    X21,
    X22,
    X23,
    X24,
    X25,
    X26,
    X27,
    X28,
    X29,
    X30,
    X31,

    REGISTER_COUNT,

    /*
     * ABI names
     */

    ZERO = X0,
    RA = X1,
    SP = X2,
    GP = X3,
    TP = X4,
    T0 = X5,
    T1 = X6,
    T2 = X7,
    FP = X8,
    S0 = X8,
    S1 = X9,
    A0 = X10,
    A1 = X11,
    A2 = X12,
    A3 = X13,
    A4 = X14,
    A5 = X15,
    A6 = X16,
    A7 = X17,
    S2 = X18,
    S3 = X19,
    S4 = X20,
    S5 = X21,
    S6 = X22,
    S7 = X23,
    S8 = X24,
    S9 = X25,
    S10 = X26,
    S11 = X27,
    T3 = X28,
    T4 = X29,
    T5 = X30,
    T6 = X31
};

enum SyscallRV : uint32_t {

    IO_SETUP = 0,
    IO_DESTROY = 1,
    IO_SUBMIT = 2,
    IO_CANCEL = 3,
    IO_GETEVENTS = 4,
    SETXATTR = 5,
    LSETXATTR = 6,
    FSETXATTR = 7,
    GETXATTR = 8,
    LGETXATTR = 9,
    FGETXATTR = 10,
    LISTXATTR = 11,
    LLISTXATTR = 12,
    FLISTXATTR = 13,
    REMOVEXATTR = 14,
    LREMOVEXATTR = 15,
    FREMOVEXATTR = 16,
    GETCWD = 17,
    LOOKUP_DCOOCKIE = 18,
    EVENTFD2 = 19,
    EPOLL_CREATE1 = 20,
    EPOLL_CTL = 21,
    EPOLL_PWAIT = 22,
    DUP = 23,
    DUP3 = 24,
    FCNTL64 = 25,
    INOTIFY_INIT1 = 26,
    INOTIFY_ADD_WATCH = 27,
    INOTIFY_RM_WATCH = 28,
    IOCTL = 29,
    IOPRIO_SET = 30,
    IOPRIO_GET = 31,
    FLOCK = 32,
    MKNODAT = 33,
    MKDIRAT = 34,
    UNLINKAT = 35,
    SYMLINKAT = 36,
    LINKAT = 37,
    RENAMEAT = 38,
    UMOUNT = 39,
    MOUNT = 40,
    PIVOT_ROOT = 41,
    NI_SYSCALL = 42,
    STATFS64 = 43,
    FSTATFS64 = 44,
    TRUNCATE64 = 45,
    FTRUNCATE64 = 46,
    FALLOCATE = 47,
    FACCESSAT = 48,
    CHDIR = 49,
    FCHDIR = 50,
    CHROOT = 51,
    FCHMOD = 52,
    FCHMODAT = 53,
    FCHOWNAT = 54,
    FCHOWN = 55,
    OPENAT = 56,
    CLOSE = 57,
    VHANGUP = 58,
    PIPE2 = 59,
    QUOTACTL = 60,
    GETDENTS64 = 61,
    LSEEK = 62,
    READ = 63,
    WRITE = 64,
    READV = 65,
    WRITEV = 66,
    PREAD64 = 67,
    PWRITE64 = 68,
    PREADV = 69,
    PWRITEV = 70,
    SENDFILE64 = 71,
    PSELECT6_TIME32 = 72,
    PPOLL_TIME32 = 73,
    SIGNALFD4 = 74,
    VMSPLICE = 75,
    SPLICE = 76,
    TEE = 77,
    READLINKAT = 78,
    NEWFSTATAT = 79,
    NEWFSTAT = 80,
    SYNC = 81,
    FSYNC = 82,
    FDATASYNC = 83,
    SYNC_FILE_RANGE = 84,
    TIMERFD_CREATE = 85,

    TIMERFD_SETTIME = 411,
    TIMERFD_GETTIME = 410,
    UTIMENSAT = 412,

    ACCT = 89,
    CAPGET = 90,
    CAPSET = 91,
    PERSONALITY = 92,
    EXIT = 93,
    EXIT_GROUP = 94,
    WAITID = 95,
    SET_TID_ADDRESS = 96,
    UNSHARE = 97,

    FUTEX = 422,

    SET_ROBUST_LIST = 99,
    GET_ROBUST_LIST = 100,
    NANOSLEEP = 101,
    GETITIMER = 102,
    SETITIMER = 103,
    KEXEC_LOAD = 104,
    INIT_MODULE = 105,
    DELETE_MODULE = 106,
    TIMER_CREATE = 107,
    TIMER_GETTIME = 108,
    TIMER_GETOVERRUN = 109,

    TIMER_SETTIME = 409,

    TIMER_DELETE = 111,

    CLOCK_SETTIME = 404,
    CLOCK_GETTIME = 403,
    CLOCK_GETRES = 406,
    CLOCK_NANOSLEEP = 407,

    SYSLOG = 116,
    PTRACE = 117,
    SCHED_SETPARAM = 118,
    SCHED_SETSCHEDULER = 119,
    SCHED_GETSCHEDULER = 120,
    SCHED_GETPARAM = 121,
    SCHED_SETAFFINITY = 122,
    SCHED_GETAFFINITY = 123,
    SCHED_YIELD = 124,
    SCHED_GET_PRIORITY_MAX = 125,
    SCHED_GET_PRIORITY_MIN = 126,

    SCHED_RR_GET_INTERVAL = 423,

    RESTART_SYSCALL = 128,
    KILL = 129,
    TKILL = 130,
    TGKILL = 131,
    SIGALTSTACK = 132,
    RT_SIGSUSPEND = 133,
    RT_SIGACTION = 134,
    RT_SIGPROCMASK = 135,
    RT_SIGPENDING = 136,
    RT_SIGTIMEDWAIT_TIME32 = 137,
    RT_SIGQUEUEINFO = 138,

    SETPRIORITY = 140,
    GETPRIORITY = 141,
    REBOOT = 142,
    SETREGID = 143,
    SETGID = 144,
    SETREUID = 145,
    SETUID = 146,
    SETRESUID = 147,
    GETRESUID = 148,
    SETRESGID = 149,
    GETRESGID = 150,
    SETFSUID = 151,
    SETFSGID = 152,
    TIMES = 153,
    SETPGID = 154,
    GETPGID = 155,
    GETSID = 156,
    SETSID = 157,
    GETGROUPS = 158,
    SETGROUPS = 159,
    NEWUNAME = 160,
    SETHOSTNAME = 161,
    SETDOMAINNAME = 162,
    GETRLIMIT = 163,
    SETRLIMIT = 164,
    GETRUSAGE = 165,
    UMASK = 166,
    PRCTL = 167,
    GETCPU = 168,
    GETTIMEOFDAY = 169,
    SETTIMEOFDAY = 170,
    ADJTIMEX = 171,
    GETPID = 172,
    GETPPID = 173,
    GETUID = 174,
    GETEUID = 175,
    GETGID = 176,
    GETEGID = 177,
    GETTID = 178,
    SYSINFO = 179,
    MQ_OPEN = 180,
    MQ_UNLINK = 181,

    MQ_TIMEDSEND = 418,
    MQ_TIMEDRECEIVE = 419,

    MQ_NOTIFY = 184,
    MQ_GETSETATTR = 185,
    MSGGET = 186,
    MSGCTL = 187,
    MSGRCV = 188,
    MSGSND = 189,
    SEMGET = 190,
    SEMCTL = 191,

    SEMTIMEDOP = 420,

    SEMOP = 193,
    SHMGET = 194,
    SHMCTL = 195,
    SHMAT = 196,
    SHMDT = 197,
    SOCKET = 198,
    SOCKETPAIR = 199,
    BIND = 200,
    LISTEN = 201,
    ACCEPT = 202,
    CONNECT = 203,
    GETSOCKNAME = 204,
    GETPEERNAME = 205,
    SENDTO = 206,
    RECVFROM = 207,
    SETSOCKOPT = 208,
    GETSOCKOPT = 209,
    SHUTDOWN = 210,
    SENDMSG = 211,
    RECVMSG = 212,
    READAHEAD = 213,
    BRK = 214,
    MUNMAP = 215,
    MREMAP = 216,
    ADD_KEY = 217,
    REQUEST_KEY = 218,
    KEYCTL = 219,
    CLONE = 220,
    EXECVE = 221,
    MMAP = 222,
    FADVISE64_64 = 223,
    SWAPON = 224,
    SWAPOFF = 225,
    MPROTECT = 226,
    MSYNC = 227,
    MLOCK = 228,
    MUNLOCK = 229,
    MLOCKALL = 230,
    MUNLOCKALL = 231,
    MINCORE = 232,
    MADVISE = 233,
    REMAP_FILE_PAGES = 234,
    MBIND = 235,
    GET_MEMPOLICY = 236,
    SET_MEMPOLICY = 237,
    MIGRATE_PAGES = 238,
    MOVE_PAGES = 239,
    RT_TGSIGQUEUEINFO = 240,
    PERF_EVENT_OPEN = 241,
    ACCEPT4 = 242,
    RECVMMSG_TIME32 = 243,

    WAIT4 = 260,
    PRLIMIT64 = 261,
    FANOTIFY_INIT = 262,
    FANOTIFY_MARK = 263,
    NAME_TO_HANDLE_AT = 264,
    OPEN_BY_HANDLE_AT = 265,

    CLOCK_ADJTIME = 405,

    SYNCFS = 267,
    SETNS = 268,
    SENDMMSG = 269,
    PROCESS_VM_READV = 270,
    PROCESS_VM_WRITEV = 271,
    KCMP = 272,
    FINIT_MODULE = 273,
    SCHED_SETATTR = 274,
    SCHED_GETATTR = 275,
    RENAMEAT2 = 276,
    SECCOMP = 277,
    GETRANDOM = 278,
    MEMFD_CREATE = 279,
    BPF = 280,
    EXECVEAT = 281,
    USERFAULTFD = 282,
    MEMBARRIER = 283,
    MLOCK2 = 284,
    COPY_FILE_RANGE = 285,
    PREADV2 = 286,
    PWRITEV2 = 287,
    PKEY_MPROTECT = 288,
    PKEY_ALLOC = 289,
    PKEY_FREE = 290,
    STATX = 291,

    IO_PGETEVENTS = 416,

    RSEQ = 293,
    KEXEC_FILE_LOAD = 294,

    PIDFD_SEND_SIGNAL = 424,
    IO_URING_SETUP = 425,
    IO_URING_ENTER = 426,
    IO_URING_REGISTER = 427,
    OPEN_TREE = 428,
    MOVE_MOUNT = 429,
    FSOPEN = 430,
    FSCONFIG = 431,
    FSMOUNT = 432,
    FSPICK = 433,
    PIDFD_OPEN = 434,
    CLONE3 = 435,
    CLOSE_RANGE = 436,
    OPENAT2 = 437,
    PIDFD_GETFD = 438,
    FACCESSAT2 = 439,
    PROCESS_MADVISE = 440
};

}  // namespace RISCV

#endif  // INCLUDE_CONSTANTS_H
