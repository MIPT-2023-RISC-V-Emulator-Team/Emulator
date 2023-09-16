#ifndef MEMORY_H
#define MEMORY_H

#include <inttypes.h>
#include <unordered_map>

#define PAGE_SIZE 			4096		// 4 KiB
#define PHYS_MEMORY_SIZE 	8589934592 	// 8 GiB
#define VIRT_MEMORY_SIZE 	8589934592 	// 8 GiB
#define PHYS_PAGE_COUNT 	PHYS_MEMORY_SIZE / PAGE_SIZE

#define ADDRESS_PAGE_NUM_SHIFT 		12
#define ADDRESS_PAGE_OFFSET_MASK 	0xFFF


namespace RISCV {


struct Page {
	uint8_t memory[PAGE_SIZE];
};


class MMU {

private:
	std::unordered_map<uint32_t, Page*> allocatedPhysPages_;

	bool allocatePage(const uint32_t pageNum);

public:
    bool load(const uint64_t addr, uint8_t* value) const;
    bool store(const uint64_t addr, uint8_t value);

    bool load32(const uint64_t addr, uint32_t* value) const;
    bool store32(const uint64_t addr, uint32_t value);

    bool loadElfFile(const std::string& filename, uint64_t* pc);

    ~MMU();
};

}

#endif // MEMORY_H
