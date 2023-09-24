#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

#include "constants.h"

namespace RISCV {

struct Register {
  uint64_t value;
};

struct EncodedInstruction {
  uint32_t instr;
};

struct DecodedInstruction {
  RegisterType rd;
  RegisterType rs1;
  RegisterType rs2;

  uint8_t immSignBitNum = 0;

  union {
    uint32_t imm = 0;
    uint32_t shamt;
  };

  InstructionType type = InstructionType::INSTRUCTION_INVALID;
};

}  // namespace RISCV

#endif  // COMMON_H
