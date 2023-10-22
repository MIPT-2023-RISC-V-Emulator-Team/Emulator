#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

#include <vector>
#include <cstdint>
#include <string>

#include "constants.h"
#include "macros.h"


namespace RISCV::memory {


struct PhysAddr {
    uint64_t pageNum;
    uint32_t pageOffset;
};


using VirtAddr = uint64_t;


class PhysicalMemory final {
private:
    uint8_t* memory_ = nullptr;
    std::vector<char> emptyPagesFlags_;

public:

    NO_COPY_SEMANTIC(PhysicalMemory);
    NO_MOVE_SEMANTIC(PhysicalMemory);

    bool allocatePage(const PhysAddr paddr);
    uint64_t getEmptyPageNumber() const;

    bool read (const PhysAddr paddr, const size_t size, void* value);
    bool write(const PhysAddr paddr, const size_t size, const void* value);

    PhysicalMemory();
    ~PhysicalMemory();
};


PhysicalMemory& getPhysicalMemory();

}  // namespace RISCV::memory

#endif  // INCLUDE_MEMORY_H
