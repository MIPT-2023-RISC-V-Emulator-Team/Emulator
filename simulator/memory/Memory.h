#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

#include <cstdint>
#include <string>
#include <vector>

#include "simulator/constants.h"
#include "utils/macros.h"

namespace RISCV::memory {

using PhysAddr = uint64_t;
using VirtAddr = uint64_t;

template <typename T>
static inline uint64_t getPageNumber(const T addr) {
    return addr >> ADDRESS_PAGE_NUM_SHIFT;
}

template <typename T>
static inline uint32_t getPageOffset(const T addr) {
    return addr & ADDRESS_PAGE_OFFSET_MASK;
}

class PhysicalMemory final {
private:
    uint8_t* memory_ = nullptr;
    std::vector<char> emptyPagesFlags_;

public:
    NO_COPY_SEMANTIC(PhysicalMemory);
    NO_MOVE_SEMANTIC(PhysicalMemory);

    bool allocatePage(const PhysAddr paddr);
    uint64_t getEmptyPageNumber() const;

    bool read(const PhysAddr paddr, const size_t size, void* value);
    bool write(const PhysAddr paddr, const size_t size, const void* value);

    PhysicalMemory();
    ~PhysicalMemory();
};

PhysicalMemory& getPhysicalMemory();

}  // namespace RISCV::memory

#endif  // INCLUDE_MEMORY_H
