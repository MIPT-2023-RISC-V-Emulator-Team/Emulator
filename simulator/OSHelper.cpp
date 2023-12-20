#include "simulator/OSHelper.h"

#include <elf.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>

namespace RISCV {

using namespace memory;

OSHelper *OSHelper::instancePtr = nullptr;

bool OSHelper::loadElfFile(Hart &hart, const std::string &filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "failed to open file \'%s\'\n", filename.c_str());
        return false;
    }

    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "ELF init failed\n");
        return false;
    }

    Elf *elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (!elf) {
        fprintf(stderr, "elf_begin() failed\n");
        return false;
    }

    if (elf_kind(elf) != ELF_K_ELF) {
        fprintf(stderr, "%s is not ELF file\n", filename.c_str());
        return false;
    }

    GElf_Ehdr ehdr;
    if (!gelf_getehdr(elf, &ehdr)) {
        fprintf(stderr, "gelf_getehdr() failed\n");
        return false;
    }

    if (gelf_getclass(elf) != ELFCLASS64) {
        fprintf(stderr, "%s must be 64-bit ELF file\n", filename.c_str());
        return false;
    }

    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        fprintf(stderr, "Cannot stat %s\n", filename.c_str());
        return false;
    }

    void *fileBuffer = mmap(NULL, fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fileBuffer == NULL) {
        fprintf(stderr, "mmap() failed\n");
        return false;
    }

    for (size_t i = 0; i < ehdr.e_phnum; ++i) {
        GElf_Phdr phdr;
        gelf_getphdr(elf, i, &phdr);

        if (phdr.p_type != PT_LOAD)
            continue;

        const MMU &translator = hart.getTranslator();

        const VirtAddr segmentStart = phdr.p_vaddr;
        const uint64_t segmentSize = phdr.p_memsz;

        heapEnd_ = std::max(heapEnd_, segmentStart + segmentSize);

        MemoryRequest request = 0;
        if (phdr.p_flags & PF_R) {
            request |= MemoryRequestBits::R;
        }
        if (phdr.p_flags & PF_W) {
            request |= MemoryRequestBits::W;
        }
        if (phdr.p_flags & PF_X) {
            request |= MemoryRequestBits::X;
        }

        // Explicitly allocate memory for those since we must take into account situation: p_memsze != p_filesz
        uint64_t vpnStart = getPageNumber(segmentStart);
        uint64_t vpnEnd = getPageNumber(segmentStart + segmentSize - 1);
        for (uint64_t vpnCurr = vpnStart; vpnCurr <= vpnEnd; ++vpnCurr) {
            translator.getPhysAddrWithAllocation(vpnCurr * PAGE_BYTESIZE, request);
        }
        if (!writeMultipaged(translator, segmentStart, phdr.p_filesz, (uint8_t *)fileBuffer + phdr.p_offset)) {
            return false;
        }
    }

    munmap(fileBuffer, fileStat.st_size);
    elf_end(elf);
    close(fd);

    heapEnd_ = heapEnd_ + (sizeof(uint64_t) - heapEnd_ & (sizeof(uint64_t) - 1));
    hart.setPC(ehdr.e_entry);
    return true;
}

bool OSHelper::allocateStack(Hart &hart, const VirtAddr stackAddr, const size_t stackSize) const {
    const MMU &translator = hart.getTranslator();
    PhysicalMemory &pmem = getPhysicalMemory();

    hart.setReg(RegisterType::SP, stackAddr);
    VirtAddr stackVAddr = stackAddr;

    const size_t stackPages = stackSize / PAGE_BYTESIZE;
    for (size_t i = 0; i < stackPages; ++i) {
        PhysAddr stackPAddr = translator.getPhysAddrWithAllocation(stackVAddr);
        if (!pmem.allocatePage(getPageNumber(stackPAddr))) {
            return false;
        }
        stackVAddr -= PAGE_BYTESIZE;
    }

    return true;
}

bool OSHelper::setupCmdArgs(Hart &hart, int argc, char **argv, char **envp) const {
    VirtAddr virtSP = hart.getReg(RegisterType::SP);
    PhysAddr physSP = 0;

    std::vector<VirtAddr> uargvPtrs;
    std::vector<VirtAddr> uenvpPtrs;

    PhysicalMemory &pmem = getPhysicalMemory();
    const MMU &translator = hart.getTranslator();

    int uargc = argc;
    int uenvc = 0;
    while (envp[uenvc] != nullptr) {
        ++uenvc;
    }

    // Put envp on the stack
    if (!putArgsStr(translator, virtSP, uenvc, envp, uenvpPtrs)) {
        return false;
    }
    virtSP = uenvpPtrs[0];

    // Put argv on the stack
    if (!putArgsStr(translator, virtSP, uargc, argv, uargvPtrs)) {
        return false;
    }
    virtSP = uargvPtrs[0];

    // Put argc on the stack
    virtSP -= sizeof(int);
    virtSP -= virtSP & 7UL;
    physSP = translator.getPhysAddr<memory::MemoryType::WMem>(virtSP);
    if (!physSP) {
        return false;
    }
    pmem.write(physSP, sizeof(uargc), &uargc);

    // Now put pointers for argv
    if (!putArgsPtr(translator, virtSP, uargvPtrs)) {
        return false;
    }
    virtSP -= sizeof(VirtAddr) * (uargc + 1);
    VirtAddr uargvStart = virtSP;

    // Now put pointers for envp
    if (!putArgsPtr(translator, virtSP, uenvpPtrs)) {
        return false;
    }
    virtSP -= sizeof(VirtAddr) * (uenvc + 1);
    VirtAddr uenvpStart = virtSP;

    virtSP -= virtSP & 0xFFF;
    hart.setReg(RegisterType::SP, virtSP);
    hart.setReg(RegisterType::A0, uargc);
    hart.setReg(RegisterType::A1, uargvStart);
    hart.setReg(RegisterType::A2, uenvpStart);

    return true;
}

bool OSHelper::writeMultipaged(const MMU &translator,
                               const VirtAddr vaddr,
                               const size_t size,
                               const uint8_t *data) const {
    PhysicalMemory &pmem = getPhysicalMemory();

    uint64_t vpnStart = getPageNumber(vaddr);
    uint64_t vpnEnd = getPageNumber(vaddr + size - 1);

    // Whole data on the same page
    if (vpnStart == vpnEnd) {
        PhysAddr paddr = translator.getPhysAddrWithAllocation(vaddr);
        if (!paddr) {
            return false;
        }
        pmem.write(paddr, size, data);
    } else {
        // Data occupies two or more pages of memory
        uint32_t copyBytesize = PAGE_BYTESIZE - getPageOffset(vaddr);
        uint32_t leftBytesize = size;
        uint32_t copiedBytesize = 0;

        PhysAddr paddr = translator.getPhysAddrWithAllocation(vaddr);
        if (!paddr) {
            return false;
        }
        pmem.write(paddr, copyBytesize, data);
        leftBytesize -= copyBytesize;
        copiedBytesize += copyBytesize;

        for (uint64_t vpnCurr = vpnStart + 1; vpnCurr <= vpnEnd; ++vpnCurr) {
            copyBytesize = std::min(PAGE_BYTESIZE, leftBytesize);

            paddr = translator.getPhysAddrWithAllocation(vpnCurr * PAGE_BYTESIZE);
            if (!paddr) {
                return false;
            }
            pmem.write(paddr, copyBytesize, data + copiedBytesize);
            leftBytesize -= copyBytesize;
            copiedBytesize += copyBytesize;
        }
    }
    return true;
}

bool OSHelper::putArgsStr(const MMU &translator,
                          memory::VirtAddr virtSP,
                          int argsCount,
                          char **args,
                          std::vector<memory::VirtAddr> &argsPtr) const {
    PhysicalMemory &pmem = getPhysicalMemory();

    argsPtr.resize(argsCount);
    for (int i = argsCount - 1; i >= 0; --i) {
        // Account for '\0'
        size_t len = std::strlen(args[i]) + 1;
        virtSP -= len;
        virtSP -= virtSP & 7UL;

        if (!writeMultipaged(translator, virtSP, len, (uint8_t *)args[i])) {
            return false;
        }
        argsPtr[i] = virtSP;
    }
    return true;
}

bool OSHelper::putArgsPtr(const MMU &translator,
                          memory::VirtAddr virtSP,
                          const std::vector<memory::VirtAddr> &argsPtr) const {
    PhysicalMemory &pmem = getPhysicalMemory();

    // NULL at the end. Already zero-ed out
    virtSP -= sizeof(VirtAddr);
    for (int i = argsPtr.size() - 1; i >= 0; --i) {
        virtSP -= sizeof(VirtAddr);
        PhysAddr physSP = translator.getPhysAddr<memory::MemoryType::WMem>(virtSP);
        if (!physSP) {
            return false;
        }
        pmem.write(physSP, sizeof(argsPtr[i]), &argsPtr[i]);
    }
    return true;
}

void OSHelper::handleSyscall(Hart *hart, const DecodedInstruction &instr) {
    const SyscallRV syscallNo = static_cast<SyscallRV>(hart->getReg(RegisterType::A7));

    switch (syscallNo) {
        case SyscallRV::READ: {
            DEBUG_INSTRUCTION("SYS_read\n");

            uint64_t fileno = hart->getReg(RegisterType::A0);
            uint64_t vaddr = hart->getReg(RegisterType::A1);
            uint64_t length = hart->getReg(RegisterType::A2);

            char outStr[1024];
            const ssize_t retVal = read(fileno, outStr, length);
            // Return number of read bytes or error
            hart->setReg(RegisterType::A0, retVal);

            if (retVal > 0) {
                memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::WMem>(vaddr);
                memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
                pmem.write(paddr, retVal, outStr);
            }
            break;
        }
        case SyscallRV::WRITE: {
            DEBUG_INSTRUCTION("SYS_write\n");

            uint64_t fileno = hart->getReg(RegisterType::A0);
            uint64_t vaddr = hart->getReg(RegisterType::A1);
            uint64_t length = hart->getReg(RegisterType::A2);

            memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

            char outStr[1024];

            memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
            pmem.read(paddr, length, outStr);

            const ssize_t retVal = write(fileno, outStr, length);

            // Return number of written bytes or error
            hart->setReg(RegisterType::A0, retVal);
            break;
        }
        case SyscallRV::EXIT: {
            DEBUG_INSTRUCTION("SYS_exit\n");

            // By specification return value is in a0, but exit status
            // passed in the same register, so just set PC to zero
            hart->setPC(0);
            break;
        }
        case SyscallRV::BRK: {
            DEBUG_INSTRUCTION("SYS_brk\n");

            VirtAddr newHeapEnd = hart->getReg(RegisterType::A0);
            if (newHeapEnd <= heapEnd_) {
                // Return break pointer
                hart->setReg(RegisterType::A0, heapEnd_);
                break;
            }

            const MMU &translator = hart->getTranslator();

            for (VirtAddr vaddr = heapEnd_; vaddr < newHeapEnd; vaddr += PAGE_BYTESIZE) {
                if (!translator.getPhysAddrWithAllocation(vaddr, MemoryRequestBits::R | MemoryRequestBits::W)) {
                    // Return error
                    hart->setReg(RegisterType::A0, -1);
                    break;
                }
            }
            if (!translator.getPhysAddrWithAllocation(newHeapEnd - 1, MemoryRequestBits::R | MemoryRequestBits::W)) {
                // Return error
                hart->setReg(RegisterType::A0, -1);
                break;
            }

            heapEnd_ = newHeapEnd;
            // Return break pointer
            hart->setReg(RegisterType::A0, heapEnd_);
            break;
        }

        case SyscallRV::IO_SETUP:
        case SyscallRV::IO_DESTROY:
        case SyscallRV::IO_SUBMIT:
        case SyscallRV::IO_CANCEL:
        case SyscallRV::IO_GETEVENTS:
        case SyscallRV::SETXATTR:
        case SyscallRV::LSETXATTR:
        case SyscallRV::FSETXATTR:
        case SyscallRV::GETXATTR:
        case SyscallRV::LGETXATTR:
        case SyscallRV::FGETXATTR:
        case SyscallRV::LISTXATTR:
        case SyscallRV::LLISTXATTR:
        case SyscallRV::FLISTXATTR:
        case SyscallRV::REMOVEXATTR:
        case SyscallRV::LREMOVEXATTR:
        case SyscallRV::FREMOVEXATTR:
        case SyscallRV::GETCWD:
        case SyscallRV::LOOKUP_DCOOCKIE:
        case SyscallRV::EVENTFD2:
        case SyscallRV::EPOLL_CREATE1:
        case SyscallRV::EPOLL_CTL:
        case SyscallRV::EPOLL_PWAIT:
        case SyscallRV::DUP:
        case SyscallRV::DUP3:
        case SyscallRV::FCNTL64:
        case SyscallRV::INOTIFY_INIT1:
        case SyscallRV::INOTIFY_ADD_WATCH:
        case SyscallRV::INOTIFY_RM_WATCH:
        case SyscallRV::IOCTL:
        case SyscallRV::IOPRIO_SET:
        case SyscallRV::IOPRIO_GET:
        case SyscallRV::FLOCK:
        case SyscallRV::MKNODAT:
        case SyscallRV::MKDIRAT:
        case SyscallRV::UNLINKAT:
        case SyscallRV::SYMLINKAT:
        case SyscallRV::LINKAT:
        case SyscallRV::RENAMEAT:
        case SyscallRV::UMOUNT:
        case SyscallRV::MOUNT:
        case SyscallRV::PIVOT_ROOT:
        case SyscallRV::NI_SYSCALL:
        case SyscallRV::STATFS64:
        case SyscallRV::FSTATFS64:
        case SyscallRV::TRUNCATE64:
        case SyscallRV::FTRUNCATE64:
        case SyscallRV::FALLOCATE:
        case SyscallRV::FACCESSAT:
        case SyscallRV::CHDIR:
        case SyscallRV::FCHDIR:
        case SyscallRV::CHROOT:
        case SyscallRV::FCHMOD:
        case SyscallRV::FCHMODAT:
        case SyscallRV::FCHOWNAT:
        case SyscallRV::FCHOWN:
        case SyscallRV::OPENAT:
        case SyscallRV::CLOSE:
        case SyscallRV::VHANGUP:
        case SyscallRV::PIPE2:
        case SyscallRV::QUOTACTL:
        case SyscallRV::GETDENTS64:
        case SyscallRV::LSEEK:
        case SyscallRV::READV:
        case SyscallRV::WRITEV:
        case SyscallRV::PREAD64:
        case SyscallRV::PWRITE64:
        case SyscallRV::PREADV:
        case SyscallRV::PWRITEV:
        case SyscallRV::SENDFILE64:
        case SyscallRV::PSELECT6_TIME32:
        case SyscallRV::PPOLL_TIME32:
        case SyscallRV::SIGNALFD4:
        case SyscallRV::VMSPLICE:
        case SyscallRV::SPLICE:
        case SyscallRV::TEE:
        case SyscallRV::READLINKAT:
        case SyscallRV::NEWFSTATAT:
        case SyscallRV::NEWFSTAT:
        case SyscallRV::SYNC:
        case SyscallRV::FSYNC:
        case SyscallRV::FDATASYNC:
        case SyscallRV::SYNC_FILE_RANGE:
        case SyscallRV::TIMERFD_CREATE:
        case SyscallRV::TIMERFD_SETTIME:
        case SyscallRV::TIMERFD_GETTIME:
        case SyscallRV::UTIMENSAT:
        case SyscallRV::ACCT:
        case SyscallRV::CAPGET:
        case SyscallRV::CAPSET:
        case SyscallRV::PERSONALITY:
        case SyscallRV::EXIT_GROUP:
        case SyscallRV::WAITID:
        case SyscallRV::SET_TID_ADDRESS:
        case SyscallRV::UNSHARE:
        case SyscallRV::FUTEX:
        case SyscallRV::SET_ROBUST_LIST:
        case SyscallRV::GET_ROBUST_LIST:
        case SyscallRV::NANOSLEEP:
        case SyscallRV::GETITIMER:
        case SyscallRV::SETITIMER:
        case SyscallRV::KEXEC_LOAD:
        case SyscallRV::INIT_MODULE:
        case SyscallRV::DELETE_MODULE:
        case SyscallRV::TIMER_CREATE:
        case SyscallRV::TIMER_GETTIME:
        case SyscallRV::TIMER_GETOVERRUN:
        case SyscallRV::TIMER_SETTIME:
        case SyscallRV::TIMER_DELETE:
        case SyscallRV::CLOCK_SETTIME:
        case SyscallRV::CLOCK_GETTIME:
        case SyscallRV::CLOCK_GETRES:
        case SyscallRV::CLOCK_NANOSLEEP:
        case SyscallRV::SYSLOG:
        case SyscallRV::PTRACE:
        case SyscallRV::SCHED_SETPARAM:
        case SyscallRV::SCHED_SETSCHEDULER:
        case SyscallRV::SCHED_GETSCHEDULER:
        case SyscallRV::SCHED_GETPARAM:
        case SyscallRV::SCHED_SETAFFINITY:
        case SyscallRV::SCHED_GETAFFINITY:
        case SyscallRV::SCHED_YIELD:
        case SyscallRV::SCHED_GET_PRIORITY_MAX:
        case SyscallRV::SCHED_GET_PRIORITY_MIN:
        case SyscallRV::SCHED_RR_GET_INTERVAL:
        case SyscallRV::RESTART_SYSCALL:
        case SyscallRV::KILL:
        case SyscallRV::TKILL:
        case SyscallRV::TGKILL:
        case SyscallRV::SIGALTSTACK:
        case SyscallRV::RT_SIGSUSPEND:
        case SyscallRV::RT_SIGACTION:
        case SyscallRV::RT_SIGPROCMASK:
        case SyscallRV::RT_SIGPENDING:
        case SyscallRV::RT_SIGTIMEDWAIT_TIME32:
        case SyscallRV::RT_SIGQUEUEINFO:
        case SyscallRV::SETPRIORITY:
        case SyscallRV::GETPRIORITY:
        case SyscallRV::REBOOT:
        case SyscallRV::SETREGID:
        case SyscallRV::SETGID:
        case SyscallRV::SETREUID:
        case SyscallRV::SETUID:
        case SyscallRV::SETRESUID:
        case SyscallRV::GETRESUID:
        case SyscallRV::SETRESGID:
        case SyscallRV::GETRESGID:
        case SyscallRV::SETFSUID:
        case SyscallRV::SETFSGID:
        case SyscallRV::TIMES:
        case SyscallRV::SETPGID:
        case SyscallRV::GETPGID:
        case SyscallRV::GETSID:
        case SyscallRV::SETSID:
        case SyscallRV::GETGROUPS:
        case SyscallRV::SETGROUPS:
        case SyscallRV::NEWUNAME:
        case SyscallRV::SETHOSTNAME:
        case SyscallRV::SETDOMAINNAME:
        case SyscallRV::GETRLIMIT:
        case SyscallRV::SETRLIMIT:
        case SyscallRV::GETRUSAGE:
        case SyscallRV::UMASK:
        case SyscallRV::PRCTL:
        case SyscallRV::GETCPU:
        case SyscallRV::GETTIMEOFDAY:
        case SyscallRV::SETTIMEOFDAY:
        case SyscallRV::ADJTIMEX:
        case SyscallRV::GETPID:
        case SyscallRV::GETPPID:
        case SyscallRV::GETUID:
        case SyscallRV::GETEUID:
        case SyscallRV::GETGID:
        case SyscallRV::GETEGID:
        case SyscallRV::GETTID:
        case SyscallRV::SYSINFO:
        case SyscallRV::MQ_OPEN:
        case SyscallRV::MQ_UNLINK:
        case SyscallRV::MQ_TIMEDSEND:
        case SyscallRV::MQ_TIMEDRECEIVE:
        case SyscallRV::MQ_NOTIFY:
        case SyscallRV::MQ_GETSETATTR:
        case SyscallRV::MSGGET:
        case SyscallRV::MSGCTL:
        case SyscallRV::MSGRCV:
        case SyscallRV::MSGSND:
        case SyscallRV::SEMGET:
        case SyscallRV::SEMCTL:
        case SyscallRV::SEMTIMEDOP:
        case SyscallRV::SEMOP:
        case SyscallRV::SHMGET:
        case SyscallRV::SHMCTL:
        case SyscallRV::SHMAT:
        case SyscallRV::SHMDT:
        case SyscallRV::SOCKET:
        case SyscallRV::SOCKETPAIR:
        case SyscallRV::BIND:
        case SyscallRV::LISTEN:
        case SyscallRV::ACCEPT:
        case SyscallRV::CONNECT:
        case SyscallRV::GETSOCKNAME:
        case SyscallRV::GETPEERNAME:
        case SyscallRV::SENDTO:
        case SyscallRV::RECVFROM:
        case SyscallRV::SETSOCKOPT:
        case SyscallRV::GETSOCKOPT:
        case SyscallRV::SHUTDOWN:
        case SyscallRV::SENDMSG:
        case SyscallRV::RECVMSG:
        case SyscallRV::READAHEAD:
        case SyscallRV::MUNMAP:
        case SyscallRV::MREMAP:
        case SyscallRV::ADD_KEY:
        case SyscallRV::REQUEST_KEY:
        case SyscallRV::KEYCTL:
        case SyscallRV::CLONE:
        case SyscallRV::EXECVE:
        case SyscallRV::MMAP:
        case SyscallRV::FADVISE64_64:
        case SyscallRV::SWAPON:
        case SyscallRV::SWAPOFF:
        case SyscallRV::MPROTECT:
        case SyscallRV::MSYNC:
        case SyscallRV::MLOCK:
        case SyscallRV::MUNLOCK:
        case SyscallRV::MLOCKALL:
        case SyscallRV::MUNLOCKALL:
        case SyscallRV::MINCORE:
        case SyscallRV::MADVISE:
        case SyscallRV::REMAP_FILE_PAGES:
        case SyscallRV::MBIND:
        case SyscallRV::GET_MEMPOLICY:
        case SyscallRV::SET_MEMPOLICY:
        case SyscallRV::MIGRATE_PAGES:
        case SyscallRV::MOVE_PAGES:
        case SyscallRV::RT_TGSIGQUEUEINFO:
        case SyscallRV::PERF_EVENT_OPEN:
        case SyscallRV::ACCEPT4:
        case SyscallRV::RECVMMSG_TIME32:
        case SyscallRV::WAIT4:
        case SyscallRV::PRLIMIT64:
        case SyscallRV::FANOTIFY_INIT:
        case SyscallRV::FANOTIFY_MARK:
        case SyscallRV::NAME_TO_HANDLE_AT:
        case SyscallRV::OPEN_BY_HANDLE_AT:
        case SyscallRV::CLOCK_ADJTIME:
        case SyscallRV::SYNCFS:
        case SyscallRV::SETNS:
        case SyscallRV::SENDMMSG:
        case SyscallRV::PROCESS_VM_READV:
        case SyscallRV::PROCESS_VM_WRITEV:
        case SyscallRV::KCMP:
        case SyscallRV::FINIT_MODULE:
        case SyscallRV::SCHED_SETATTR:
        case SyscallRV::SCHED_GETATTR:
        case SyscallRV::RENAMEAT2:
        case SyscallRV::SECCOMP:
        case SyscallRV::GETRANDOM:
        case SyscallRV::MEMFD_CREATE:
        case SyscallRV::BPF:
        case SyscallRV::EXECVEAT:
        case SyscallRV::USERFAULTFD:
        case SyscallRV::MEMBARRIER:
        case SyscallRV::MLOCK2:
        case SyscallRV::COPY_FILE_RANGE:
        case SyscallRV::PREADV2:
        case SyscallRV::PWRITEV2:
        case SyscallRV::PKEY_MPROTECT:
        case SyscallRV::PKEY_ALLOC:
        case SyscallRV::PKEY_FREE:
        case SyscallRV::STATX:
        case SyscallRV::IO_PGETEVENTS:
        case SyscallRV::RSEQ:
        case SyscallRV::KEXEC_FILE_LOAD:
        case SyscallRV::PIDFD_SEND_SIGNAL:
        case SyscallRV::IO_URING_SETUP:
        case SyscallRV::IO_URING_ENTER:
        case SyscallRV::IO_URING_REGISTER:
        case SyscallRV::OPEN_TREE:
        case SyscallRV::MOVE_MOUNT:
        case SyscallRV::FSOPEN:
        case SyscallRV::FSCONFIG:
        case SyscallRV::FSMOUNT:
        case SyscallRV::FSPICK:
        case SyscallRV::PIDFD_OPEN:
        case SyscallRV::CLONE3:
        case SyscallRV::CLOSE_RANGE:
        case SyscallRV::OPENAT2:
        case SyscallRV::PIDFD_GETFD:
        case SyscallRV::FACCESSAT2:
        case SyscallRV::PROCESS_MADVISE: {
            std::cerr << "unimplemented syscall was called" << std::endl;
            break;
        }

        default: {
            std::cerr << "unknown syscall" << std::endl;
            break;
        }
    }
}

}  // namespace RISCV
