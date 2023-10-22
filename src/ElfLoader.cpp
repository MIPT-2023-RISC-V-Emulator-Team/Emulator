#include <elf.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#include "ElfLoader.h"


namespace RISCV {

using namespace memory;

ElfLoader* ElfLoader::instancePtr = nullptr;


bool ElfLoader::loadElf(const std::string& filename, VirtAddr& outEntry) {
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

        MemoryTranslator translator;
        PhysicalMemory* pmem = PhysicalMemory::getInstance();

        const uint64_t segmentStart = phdr.p_vaddr;
        const uint64_t segmentSize = phdr.p_memsz;

        const PhysAddr paddrStart = translator.getPhysAddr(segmentStart);
        const PhysAddr paddrEnd   = translator.getPhysAddr(segmentStart + segmentSize - 1);

        // If segment occupies only one page of memory
        if (paddrStart.pageNum == paddrEnd.pageNum) {
            if (!pmem->allocatePage(paddrStart)) {
                munmap(fileBuffer, fileStat.st_size);
                elf_end(elf);
                close(fd);
                return false;
            }

            Page* page = pmem->getPage(paddrStart);
            if(!page) {
                munmap(fileBuffer, fileStat.st_size);
                elf_end(elf);
                close(fd);
                return false;                
            }

            memcpy(page->memory.data() + paddrStart.pageOffset,
                   (uint8_t*)fileBuffer + phdr.p_offset,
                   phdr.p_filesz);
            continue;
        }

        // If segment occupies two or more pages of memory
        uint32_t pageOffset = paddrStart.pageOffset;
        uint32_t copyBytesize = PAGE_BYTESIZE - pageOffset;
        uint32_t leftBytesize = phdr.p_filesz;
        uint32_t fileOffset = phdr.p_offset;

        for (uint32_t currPageNum = paddrStart.pageNum; currPageNum <= paddrEnd.pageNum; ++currPageNum) {
            Page* currPage = pmem->getPage(currPageNum);
            if (!currPage) {
                PhysAddr tmp;
                tmp.pageNum = currPageNum;
                tmp.pageOffset = 0;
                if (!pmem->allocatePage(tmp)) {
                    munmap(fileBuffer, fileStat.st_size);
                    elf_end(elf);
                    close(fd);
                    return false;                    
                }
                currPage = pmem->getPage(currPageNum);
            }

            memcpy(currPage->memory.data() + pageOffset,
                   (uint8_t*)fileBuffer + fileOffset,
                   copyBytesize);

            fileOffset += copyBytesize;
            leftBytesize -= copyBytesize;

            pageOffset = 0;
            copyBytesize = std::min((uint32_t)PAGE_BYTESIZE, leftBytesize);
        }
    }

    munmap(fileBuffer, fileStat.st_size);
    elf_end(elf);
    close(fd);

    outEntry = ehdr.e_entry;

    printf("Loaded ELF file: %s\n\n", filename.c_str());
    return true;
}

}   // namespace RISCV
