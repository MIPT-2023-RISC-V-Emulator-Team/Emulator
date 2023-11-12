#include "ElfLoader.h"

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

ElfLoader* ElfLoader::instancePtr = nullptr;

bool ElfLoader::loadElf(const std::string& filename, Hart& hart) {
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

        MemoryRequest request;
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

}  // namespace RISCV
