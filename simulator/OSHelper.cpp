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

OSHelper* OSHelper::instancePtr = nullptr;

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

    Elf* elf = elf_begin(fd, ELF_C_READ, nullptr);
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

    void* fileBuffer = mmap(NULL, fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fileBuffer == NULL) {
        fprintf(stderr, "mmap() failed\n");
        return false;
    }

    for (size_t i = 0; i < ehdr.e_phnum; ++i) {
        GElf_Phdr phdr;
        gelf_getphdr(elf, i, &phdr);

        if (phdr.p_type != PT_LOAD)
            continue;

        const MMU& translator = hart.getTranslator();
        PhysicalMemory& pmem = getPhysicalMemory();

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

        const PhysAddr paddrStart = translator.getPhysAddrWithAllocation(segmentStart, request);
        const PhysAddr paddrEnd =
            translator.getPhysAddrWithAllocation(segmentStart + segmentSize - 1, request);

        // If segment occupies only one page of memory
        if (getPageNumber(paddrStart) == getPageNumber(paddrEnd)) {
            // Use phdr.p_filesz because remaining information will be filled with zeros as supposed
            // to be
            if (!pmem.write(paddrStart, phdr.p_filesz, (uint8_t*)fileBuffer + phdr.p_offset)) {
                munmap(fileBuffer, fileStat.st_size);
                elf_end(elf);
                close(fd);
                return false;
            }
            continue;
        }

        // If segment occupies two or more pages of memory
        uint32_t copyBytesize = PAGE_BYTESIZE - getPageOffset(segmentStart);
        uint32_t leftBytesize = phdr.p_filesz;
        uint32_t fileOffset = phdr.p_offset;

        size_t addressIncrementSize = copyBytesize;
        for (VirtAddr currVirtAddr = segmentStart; currVirtAddr < segmentStart + segmentSize;) {
            PhysAddr currPhysAddr = translator.getPhysAddrWithAllocation(currVirtAddr, request);
            pmem.write(currPhysAddr, copyBytesize, (uint8_t*)fileBuffer + fileOffset);

            fileOffset += copyBytesize;
            leftBytesize -= copyBytesize;

            copyBytesize = std::min((uint32_t)PAGE_BYTESIZE, leftBytesize);

            /*
             * Explicitly define loop ending since we need to add <PAGE_BYTESIZE -
             * getPageOffset(segmentStart)> to virtual address only once and add <PAGE_BYTESIZE> all
             * other times
             */
            currVirtAddr += addressIncrementSize;
            addressIncrementSize = PAGE_BYTESIZE;
        }
    }

    munmap(fileBuffer, fileStat.st_size);
    elf_end(elf);
    close(fd);

    hart.setPC(ehdr.e_entry);
    return true;
}


bool OSHelper::allocateStack(Hart &hart, const VirtAddr stackAddr, const size_t stackSize) {
    const MMU& translator = hart.getTranslator();
    PhysicalMemory& pmem = getPhysicalMemory();

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


bool OSHelper::setupCmdArgs(Hart &hart, int argc, char **argv, char **envp) {
    VirtAddr virtSP = hart.getReg(RegisterType::SP);
    PhysAddr physSP = 0;

    std::vector<VirtAddr> uargv;
    std::vector<VirtAddr> uenvp;

    VirtAddr uargvStart = 0;
    VirtAddr uenvpStart = 0;

    PhysicalMemory& pmem = getPhysicalMemory();
    const MMU &translator = hart.getTranslator();

    int uargc = argc - 1;
    int uenvc = 0;
    while (envp[uenvc] != nullptr) {
        ++uenvc;
    }

    // Put envp on the stack
    uenvp.resize(uenvc);
    for (int i = uenvc - 1; i >= 0; --i) {

        // Account for '\0'
        size_t len = std::strlen(envp[i]) + 1;
        virtSP -= len;
        virtSP -= virtSP & 7UL;

        uint64_t vpnStart = getPageNumber(virtSP);
        uint64_t vpnEnd = getPageNumber(virtSP + len);

        // Whole string on the same page
        if (vpnStart == vpnEnd) {
            physSP = translator.getPhysAddrWithAllocation(virtSP);
            pmem.write(physSP, len, envp[i]);
        }
        else {
            // String occupies two or more pages of memory
            uint32_t copyBytesize = PAGE_BYTESIZE - getPageOffset(virtSP);
            uint32_t leftBytesize = len;
            uint32_t copiedBytesize = 0;

            physSP = translator.getPhysAddrWithAllocation(virtSP);
            pmem.write(physSP, copyBytesize, envp[i]);
            leftBytesize -= copyBytesize;
            copiedBytesize += copyBytesize;

            for (uint64_t vpnCurr = vpnStart + 1; vpnCurr <= vpnEnd; ++vpnCurr) {
                copyBytesize = std::min(PAGE_BYTESIZE, leftBytesize);

                physSP = translator.getPhysAddrWithAllocation(vpnCurr * PAGE_BYTESIZE);
                pmem.write(physSP, copyBytesize, envp[i] + copiedBytesize);
                leftBytesize -= copyBytesize;
                copiedBytesize += copyBytesize;
            }
        }
        uenvp[i] = virtSP;
    }

    // Put argv on the stack
    uargv.resize(uargc);
    for (int i = uargc - 1; i >= 0; --i) {

        // Account for '\0'
        size_t len = std::strlen(argv[i + 1]) + 1;
        virtSP -= len;
        virtSP -= virtSP & 7UL;

        uint64_t vpnStart = getPageNumber(virtSP);
        uint64_t vpnEnd = getPageNumber(virtSP + len);

        // Whole string on the same page
        if (vpnStart == vpnEnd) {
            physSP = translator.getPhysAddrWithAllocation(virtSP);
            pmem.write(physSP, len, argv[i + 1]);
        }
        else {
            // String occupies two or more pages of memory
            uint32_t copyBytesize = PAGE_BYTESIZE - getPageOffset(virtSP);
            uint32_t leftBytesize = len;
            uint32_t copiedBytesize = 0;

            physSP = translator.getPhysAddrWithAllocation(virtSP);
            pmem.write(physSP, copyBytesize, argv[i + 1]);
            leftBytesize -= copyBytesize;
            copiedBytesize += copyBytesize;

            for (uint64_t vpnCurr = vpnStart + 1; vpnCurr <= vpnEnd; ++vpnCurr) {
                copyBytesize = std::min(PAGE_BYTESIZE, leftBytesize);

                physSP = translator.getPhysAddrWithAllocation(vpnCurr * PAGE_BYTESIZE);
                pmem.write(physSP, copyBytesize, argv[i + 1] + copiedBytesize);
                leftBytesize -= copyBytesize;
                copiedBytesize += copyBytesize;
            }
        }
        uargv[i] = virtSP;
    }

    // Put argc on the stack
    virtSP -= sizeof(int);
    virtSP -= virtSP & 7UL;
    physSP = translator.getPhysAddrWithAllocation(virtSP);
    pmem.write(physSP, sizeof(uargc), &uargc);


    // Now put pointers for argv

    // NULL at the end. Already zero-ed out
    virtSP -= sizeof(VirtAddr);
    for (int i = uargc - 1; i >= 0; --i) {
        virtSP -= sizeof(VirtAddr);
        physSP = translator.getPhysAddrWithAllocation(virtSP);
        pmem.write(physSP, sizeof(uargv[i]), &uargv[i]);
    }
    uargvStart = virtSP;


    // Now put pointers for envp

    // NULL at the end. Already zero-ed out
    virtSP -= sizeof(VirtAddr);
    for (int i = uenvc - 1; i >= 0; --i) {
        virtSP -= sizeof(VirtAddr);
        physSP = translator.getPhysAddrWithAllocation(virtSP);
        pmem.write(physSP, sizeof(uenvp[i]), &uenvp[i]);
    }
    uenvpStart = virtSP;


    virtSP -= virtSP & 0xFFF;
    hart.setReg(RegisterType::SP, virtSP);
    hart.setReg(RegisterType::A0, uargc);
    hart.setReg(RegisterType::A1, uargvStart);
    hart.setReg(RegisterType::A2, uenvpStart);

    return true;
}

}  // namespace RISCV
