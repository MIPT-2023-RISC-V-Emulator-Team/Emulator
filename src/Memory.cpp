#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include <elf.h>
#include <gelf.h>
#include <libelf.h>

#include <cstring>

#include "../include/Memory.h"

namespace RISCV {


bool MMU::allocatePage(const uint32_t pageNum) {
    if (allocatedPhysPages_.size() >= PHYS_PAGE_COUNT) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    allocatedPhysPages_[pageNum] = new Page();
    return true;
}


bool MMU::load8(const uint64_t addr, uint8_t* value) const {
    uint32_t pageNum = addr >> ADDRESS_PAGE_NUM_SHIFT;
    
    if (auto it = allocatedPhysPages_.find(pageNum); it != allocatedPhysPages_.end()) {
        uint32_t offset = addr & ADDRESS_PAGE_OFFSET_MASK;

        *value = it->second->memory[offset];
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}


bool MMU::load16(const uint64_t addr, uint16_t* value) const {
    uint8_t part0;
    uint8_t part1;

    if (!load8(addr, &part0) || !load8(addr + 1, &part1)) {
        return false;
    }

    *value = (part0 << 0) | (((uint16_t)part1) << 8);
    return true;
}


bool MMU::load32(const uint64_t addr, uint32_t* value) const {
    uint16_t part0;
    uint16_t part1;

    if (!load16(addr, &part0) || !load16(addr + 2, &part1)) {
        return false;
    }

    *value = (part0 << 0) | (((uint32_t)part1) << 16);
    return true;
}


bool MMU::load64(const uint64_t addr, uint64_t* value) const {
    uint32_t part0;
    uint32_t part1;

    if (!load32(addr, &part0) || !load32(addr + 4, &part1)) {
        return false;
    }

    *value = (part0 << 0) | (((uint64_t)part1) << 32);
    return true;
}



bool MMU::store8(const uint64_t addr, uint8_t value) {
    uint32_t pageNum = addr >> ADDRESS_PAGE_NUM_SHIFT;
    uint32_t offset = addr & ADDRESS_PAGE_OFFSET_MASK;

    if (auto it = allocatedPhysPages_.find(pageNum); it != allocatedPhysPages_.end()) {

        it->second->memory[offset] = value;
        return true;
    }

    if (!allocatePage(pageNum))
        return false;

    allocatedPhysPages_[pageNum]->memory[offset] = value;
    return true;
}


bool MMU::store16(const uint64_t addr, uint16_t value) {
    if (!store8(addr, value & 0x00FF) || !store8(addr + 1, value << 8)) {
        return false;
    }
    return true;
}


bool MMU::store32(const uint64_t addr, uint32_t value) {
    if (!store16(addr, value & 0x0000FFFF) || !store16(addr + 2, value << 16)) {
        return false;
    }
    return true;
}


bool MMU::store64(const uint64_t addr, uint64_t value) {
    if (!store32(addr, value & 0x00000000FFFFFFFF) || !store32(addr + 4, value << 32)) {
        return false;
    }
    return true;
}



bool MMU::loadElfFile(const std::string& filename, uint64_t* pc) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "failed to open file \'%s\'\n", filename.c_str());
        return -1;
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
        return -1;
    }

    struct stat elf_file_stat;
    if (fstat(fd, &elf_file_stat) == -1) {
        fprintf(stderr, "Cannot stat %s\n", filename.c_str());
        return false;
    }

    void *elf_file_buffer = mmap(NULL, elf_file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (elf_file_buffer == NULL) {
        fprintf(stderr, "mmap() failed\n");
        return false;
    }

    for (size_t i = 0; i < ehdr.e_phnum; ++i) {
        GElf_Phdr phdr; 
        gelf_getphdr(elf, i, &phdr);

        if (phdr.p_type != PT_LOAD)
            continue;

        const uint64_t segmentStart = phdr.p_vaddr;
        const uint64_t segmentSize  = phdr.p_filesz;

        const uint32_t pageNumStart = segmentStart >> ADDRESS_PAGE_NUM_SHIFT;
        const uint32_t pageNumEnd   = (segmentStart + segmentSize) >> ADDRESS_PAGE_NUM_SHIFT;


        // Only one page of memory
        if (pageNumStart == pageNumEnd) {
            if (allocatedPhysPages_.find(pageNumStart) == allocatedPhysPages_.end())
                if (!allocatePage(pageNumStart)) {
                    munmap(elf_file_buffer, elf_file_stat.st_size);
                    elf_end(elf);
                    close(fd);
                    return false;
                }

            uint32_t offset = segmentStart & ADDRESS_PAGE_OFFSET_MASK;
            memcpy(allocatedPhysPages_[pageNumStart]->memory + offset, (uint8_t*)elf_file_buffer + phdr.p_offset, phdr.p_filesz);

            *pc = ehdr.e_entry;
            break;
        }

        // TODO: Make allocation and copying for multiple pages
    }

    munmap(elf_file_buffer, elf_file_stat.st_size);
    elf_end(elf);
    close(fd);
    return true;
}


MMU::~MMU() {
    for (auto page : allocatedPhysPages_) {
        delete page.second;
    }
}

}
