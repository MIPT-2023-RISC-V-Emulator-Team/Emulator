#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "simulator/constants.h"
#include "utils/macros.h"

namespace RISCV::memory {

enum class MemoryType : uint8_t { IMem, RMem, WMem };

using PhysAddr = uint64_t;
using VirtAddr = uint64_t;

template <typename T>
static inline uint64_t getPageNumber(const T addr) {
    return addr >> ADDRESS_PAGE_NUM_SHIFT;
}

template <typename T>
static inline uint64_t getPageNumberUnshifted(const T addr) {
    return addr & ~ADDRESS_PAGE_OFFSET_MASK;
}

template <typename T>
static inline uint32_t getPageOffset(const T addr) {
    return addr & ADDRESS_PAGE_OFFSET_MASK;
}

class PhysicalMemory final {
private:
    uint8_t *memory_ = nullptr;
    std::vector<char> emptyPagesFlags_;
    std::vector<uint32_t> allocatedPages_;

public:
    NO_COPY_SEMANTIC(PhysicalMemory);
    NO_MOVE_SEMANTIC(PhysicalMemory);

    bool allocatePage(const uint64_t pageNum);
    bool freePage(const uint64_t pageNum);
    void freeAllPages();
    uint64_t getEmptyPageNumber() const;

    inline bool read(const PhysAddr paddr, const size_t size, void *value) {
        std::memcpy(value, memory_ + paddr, size);
        return true;
    }

    inline bool write(const PhysAddr paddr, const size_t size, const void *value) {
        std::memcpy(memory_ + paddr, value, size);
        return true;
    }

    PhysicalMemory();
    ~PhysicalMemory();
};

PhysicalMemory &getPhysicalMemory();

}  // namespace RISCV::memory

#endif  // INCLUDE_MEMORY_H
