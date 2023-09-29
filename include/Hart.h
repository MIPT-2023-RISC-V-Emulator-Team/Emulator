#ifndef INCLUDE_HART_H
#define INCLUDE_HART_H

#include <array>

#include "Common.h"
#include "Memory.h"

namespace RISCV {

class Hart {
 public:
  void fetch(EncodedInstruction& encInstr);
  void decode(const EncodedInstruction encInstr, DecodedInstruction& decInstr) const;
  void execute(const DecodedInstruction& decInstr);

  void loadElfFile(const std::string& filename) {
    m_mmu.loadElfFile(filename, &m_pc);
  }

  uint64_t getPC() const {
    return m_pc;
  };

  Hart() {
    m_regs[RegisterType::SP] = m_mmu.getStackAddress();
  };

 private:
  memory::MMU m_mmu;
  uint64_t m_pc;
  std::array<RegValue, RegisterType::REGISTER_COUNT> m_regs = {};
};

}  // namespace RISCV

#endif  // INCLUDE_HART_H
