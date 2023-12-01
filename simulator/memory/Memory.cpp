#include <simulator/memory/Memory.h>

#include <algorithm>
#include <cstring>
#include <string>

#include <simulator/constants.h>

namespace RISCV::memory {

PhysicalMemory g_physicalMemory;

bool PhysicalMemory::allocatePage(const uint64_t pageNum) {
    if (pageNum >= PHYS_PAGE_COUNT) {
        std::cerr << "invalid address" << std::endl;
        return false;
    }

    if (std::find(allocatedPages_.begin(), allocatedPages_.end(), pageNum) ==
        allocatedPages_.end()) {
        allocatedPages_.push_back(pageNum);
        emptyPagesFlags_[pageNum] = 0;
    }
    return true;
}

bool PhysicalMemory::freePage(const uint64_t pageNum) {
    if (pageNum >= PHYS_PAGE_COUNT) {
        std::cerr << "invalid address" << std::endl;
        return false;
    }

    if (auto it = std::find(allocatedPages_.begin(), allocatedPages_.end(), pageNum); it != allocatedPages_.end()) {
        allocatedPages_.erase(it);
    }
    emptyPagesFlags_[pageNum] = 1;
    return true;
}

void PhysicalMemory::freeAllPages() {
    allocatedPages_.clear();
    std::fill(emptyPagesFlags_.begin(), emptyPagesFlags_.end(), 1);
    std::fill(memory_, memory_ + PHYS_MEMORY_BYTESIZE, 0);
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

PhysicalMemory::PhysicalMemory() : emptyPagesFlags_(PHYS_PAGE_COUNT, 1) {
    memory_ = new uint8_t[PHYS_MEMORY_BYTESIZE]{};
}

PhysicalMemory::~PhysicalMemory() {
    delete[] memory_;
}

PhysicalMemory& getPhysicalMemory() {
    return g_physicalMemory;
}

}  // namespace RISCV::memory
