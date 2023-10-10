#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

#include "Executor.h"
#include "constants.h"

namespace RISCV {

using RegValue = uint64_t;
using SignedRegValue = int64_t;

using Executor = void (*)(Hart* hart, const DecodedInstruction& instr);

using EncodedInstruction = uint32_t;

struct DecodedInstruction {
    RegisterType rd;
    RegisterType rs1;
    RegisterType rs2;

    uint8_t immSignBitNum = 0;  // Helper to sext

    union {
        uint32_t imm = 0;
        uint32_t shamt;
    };

    Executor exec;

    InstructionType type = InstructionType::INSTRUCTION_INVALID;
};

}  // namespace RISCV

#endif  // COMMON_H
