#include "Memory.h"

#include <string>

#include "constants.h"


namespace RISCV::memory {

PhysicalMemory* PhysicalMemory::instancePtr = nullptr;


bool PhysicalMemory::load8(const PhysAddr paddr, uint8_t* value) const {
    if (paddr.pageOffset > PAGE_BYTESIZE - 1) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *value = it->second->memory[paddr.pageOffset];
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}

bool PhysicalMemory::load16(const PhysAddr paddr, uint16_t* value) const {
    if (paddr.pageOffset > PAGE_BYTESIZE - 2) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *value = *reinterpret_cast<uint16_t*>(&it->second->memory[paddr.pageOffset]);
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}

bool PhysicalMemory::load32(const PhysAddr paddr, uint32_t* value) const {
    if (paddr.pageOffset > PAGE_BYTESIZE - 4) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *value = *reinterpret_cast<uint32_t*>(&it->second->memory[paddr.pageOffset]);
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}

bool PhysicalMemory::load64(const PhysAddr paddr, uint64_t* value) const {
    if (paddr.pageOffset > PAGE_BYTESIZE - 8) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *value = *reinterpret_cast<uint64_t*>(&it->second->memory[paddr.pageOffset]);
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}


bool PhysicalMemory::store8(const PhysAddr paddr, uint8_t value) {
    if (paddr.pageOffset > PAGE_BYTESIZE - 1) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        it->second->memory[paddr.pageOffset] = value;
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}

bool PhysicalMemory::store16(const PhysAddr paddr, uint16_t value) {
    if (paddr.pageOffset > PAGE_BYTESIZE - 2) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *reinterpret_cast<uint16_t*>(&it->second->memory[paddr.pageOffset]) = value;
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}

bool PhysicalMemory::store32(const PhysAddr paddr, uint32_t value) {
    if (paddr.pageOffset > PAGE_BYTESIZE - 4) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *reinterpret_cast<uint32_t*>(&it->second->memory[paddr.pageOffset]) = value;
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}

bool PhysicalMemory::store64(const PhysAddr paddr, uint64_t value) {
    if (paddr.pageOffset > PAGE_BYTESIZE - 8) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (auto it = allocatedPages_.find(paddr.pageNum); it != allocatedPages_.end()) {
        *reinterpret_cast<uint64_t*>(&it->second->memory[paddr.pageOffset]) = value;
        return true;
    }
    fprintf(stderr, "PAGE FAULT OCCURS\n");
    return false;
}


bool PhysicalMemory::allocatePage(const PhysAddr paddr) {
    if (paddr.pageNum >= PHYS_PAGE_COUNT) {
        fprintf(stderr, "PAGE FAULT OCCURS\n");
        return false;
    }

    if (allocatedPages_.find(paddr.pageNum) == allocatedPages_.cend()) {
        allocatedPages_[paddr.pageNum] = new Page();
    }
    return true;
}


Page* PhysicalMemory::getPage(const uint32_t pageNum) {
    if (auto it = allocatedPages_.find(pageNum); it != allocatedPages_.end()) {
        return it->second;
    }
    return nullptr;
}


Page* PhysicalMemory::getPage(const PhysAddr paddr) {
    return getPage(paddr.pageNum);
}


PhysAddr MemoryTranslator::getPhysAddr(const VirtAddr vaddr) const {
    PhysAddr paddr;
    paddr.pageNum = vaddr >> ADDRESS_PAGE_NUM_SHIFT;
    paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
    return paddr;
}


MMU::MMU() {
    PhysicalMemory* pmem = PhysicalMemory::getInstance();
    PhysAddr stackPAddr = getPhysAddr(DEFAULT_STACK_ADDRESS);
    pmem->allocatePage(stackPAddr);
}


}  // namespace RISCV::memory
