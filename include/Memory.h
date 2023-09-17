#ifndef MEMORY_H
#define MEMORY_H

#include <inttypes.h>
#include <unordered_map>

#define PAGE_BYTESIZE 			4096		    // 4 KiB
#define PHYS_MEMORY_BYTESIZE 	8589934592 	    // 8 GiB
#define VIRT_MEMORY_BYTESIZE 	8589934592      // 8 GiB
#define PHYS_PAGE_COUNT 	    PHYS_MEMORY_BYTESIZE / PAGE_BYTESIZE

#define ADDRESS_PAGE_NUM_SHIFT 		12
#define ADDRESS_PAGE_OFFSET_MASK 	0xFFF

#define STACK_BYTESIZE          1048576     // 1 MiB
#define DEFAULT_STACK_ADDRESS   VIRT_MEMORY_BYTESIZE - STACK_BYTESIZE

namespace RISCV {


struct Page {
	uint8_t memory[PAGE_BYTESIZE];
};


class MMU {

private:
	std::unordered_map<uint32_t, Page*> allocatedPhysPages_;
    uint64_t stackAddress_ = DEFAULT_STACK_ADDRESS;

	bool allocatePage(const uint32_t pageNum);

public:
    uint64_t getStackAddress() const;

    bool load8 (const uint64_t addr,  uint8_t* value) const;
    bool load16(const uint64_t addr, uint16_t* value) const;
    bool load32(const uint64_t addr, uint32_t* value) const;
    bool load64(const uint64_t addr, uint64_t* value) const;

    bool store8 (const uint64_t addr,  uint8_t value);
    bool store16(const uint64_t addr, uint16_t value);
    bool store32(const uint64_t addr, uint32_t value);
    bool store64(const uint64_t addr, uint64_t value);

    bool loadElfFile(const std::string& filename, uint64_t* pc);

    MMU();
    ~MMU();
};

}

#endif // MEMORY_H
