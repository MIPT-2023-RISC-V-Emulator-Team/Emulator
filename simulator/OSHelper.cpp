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

bool OSHelper::loadElfFile(Hart &hart, const std::string &filename) const {
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

}  // namespace RISCV
