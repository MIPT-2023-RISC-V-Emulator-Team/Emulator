#include "Memory.h"

#include <string>
#include <algorithm>
#include <cstring>

#include "constants.h"


namespace RISCV::memory {

PhysicalMemory g_physicalMemory;


bool PhysicalMemory::allocatePage(const PhysAddr paddr) {
    uint64_t pageNum = getPageNumber(paddr);
    if (pageNum >= PHYS_PAGE_COUNT) {
        std::cerr << "invalid address" << std::endl;
        return false;
    }

    static std::vector<uint32_t> allocatedPages_;

    if (std::find(allocatedPages_.begin(), allocatedPages_.end(), pageNum) == allocatedPages_.end()) {
        allocatedPages_.push_back(pageNum);
        emptyPagesFlags_[pageNum] = 0;
    }
    return true;
}

uint64_t PhysicalMemory::getEmptyPageNumber() const {
    auto it = std::find(emptyPagesFlags_.begin(), emptyPagesFlags_.end(), 1);
    return it - emptyPagesFlags_.begin();
}


bool PhysicalMemory::read(const PhysAddr paddr, const size_t size, void* value) {
    std::memcpy(value, memory_ + paddr, size);
    return true;
}


bool PhysicalMemory::write(const PhysAddr paddr, const size_t size, const void* value) {
    std::memcpy(memory_ + paddr, value, size);
    return true;
}


PhysicalMemory::PhysicalMemory() {
    memory_ = new uint8_t[PHYS_MEMORY_BYTESIZE];
    emptyPagesFlags_.resize(PHYS_PAGE_COUNT);
    std::fill(emptyPagesFlags_.begin(), emptyPagesFlags_.end(), 1);
}

PhysicalMemory::~PhysicalMemory() {
    delete[] memory_;
}


PhysicalMemory& getPhysicalMemory() {
    return g_physicalMemory;
}


}  // namespace RISCV::memory
