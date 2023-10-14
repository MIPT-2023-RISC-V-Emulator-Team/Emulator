#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

#include "Executor.h"
#include "constants.h"
#include "generated/InstructionTypes.h"

namespace RISCV {

using RegValue = uint64_t;
using SignedRegValue = int64_t;

using Executor = void (*)(Hart* hart, const DecodedInstruction& instr);

using EncodedInstruction = uint32_t;

struct DecodedInstruction {
    RegisterType rd;
    RegisterType rs1;
    RegisterType rs2;
    RegisterType rs3;
    RegisterType rm;

    uint8_t pred = 0;
    uint8_t succ = 0;
    uint8_t fm = 0;

    uint8_t immSignBitNum = 0;  // Helper to sext

    union {
        uint32_t imm = 0;
        uint32_t shamt;
        uint32_t shamtw;
        uint32_t aqrl;
    };

    Executor exec;

    // Initialize instruction to be invalid so every return will
    // be either valid instruction or invalid one
    InstructionType type = InstructionType::INSTRUCTION_INVALID;
};

}  // namespace RISCV

#endif  // COMMON_H
