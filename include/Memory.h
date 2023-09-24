#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "constants.h"

namespace RISCV::memory {

struct Page {
  std::array<uint8_t, memory::PAGE_BYTESIZE> memory;
};

class MMU {
 public:
  MMU();
  ~MMU();

  bool loadElfFile(const std::string& filename, uint64_t* pc);
  uint64_t getStackAddress() const;

  bool load8(const uint64_t addr, uint8_t* value) const;
  bool load16(const uint64_t addr, uint16_t* value) const;
  bool load32(const uint64_t addr, uint32_t* value) const;
  bool load64(const uint64_t addr, uint64_t* value) const;

  bool store8(const uint64_t addr, uint8_t value);
  bool store16(const uint64_t addr, uint16_t value);
  bool store32(const uint64_t addr, uint32_t value);
  bool store64(const uint64_t addr, uint64_t value);

 private:
  bool allocatePage(const uint32_t pageNum);

  std::unordered_map<uint32_t, Page*> allocatedPhysPages_;
  uint64_t stackAddress_ = DEFAULT_STACK_ADDRESS;
};

}  // namespace RISCV::memory

#endif  // INCLUDE_MEMORY_H
