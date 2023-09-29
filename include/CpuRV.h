#ifndef INCLUDE_CPU_RV_H
#define INCLUDE_CPU_RV_H

#include <array>

#include "Common.h"
#include "Memory.h"

namespace RISCV {

class CpuRV {
 public:
  void fetch(EncodedInstruction& encInstr);
  void decode(const EncodedInstruction& encInstr, DecodedInstruction& decInstr) const;
  void execute(const DecodedInstruction& decInstr);

  void loadElfFile(const std::string& filename) {
    mmu_.loadElfFile(filename, &pc_);
  }

  uint64_t getPC() const {
    return pc_;
  };

  CpuRV() {
    regs_[RegisterType::SP].value = mmu_.getStackAddress();
  };

 private:
  memory::MMU mmu_;
  uint64_t pc_;
  std::array<Register, RegisterType::REGISTER_COUNT> regs_ = {};
};

}  // namespace RISCV

#endif  // INCLUDE_CPU_RV_H
