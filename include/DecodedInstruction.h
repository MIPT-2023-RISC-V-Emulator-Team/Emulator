#ifndef INCLUDE_DECODE_INSCTRUCTION_H
#define INCLUDE_DECODE_INSCTRUCTION_H

#include "constants.h"
#include "generated/InstructionTypes.h"

namespace RISCV {

struct DecodedInstruction {
    bool isJumpInstruction() {
        switch (type) {
            case InstructionType::JAL:
            case InstructionType::JALR:
            case InstructionType::BEQ:
            case InstructionType::BNE:
            case InstructionType::BLT:
            case InstructionType::BGE:
            case InstructionType::BLTU:
            case InstructionType::BGEU:
                return true;
            default:
                return false;
        }
    }

    RegisterType rd;
    RegisterType rs1;

    // Unimplemented yet
    union {
        RegisterType rs2;
        RegisterType rs3;
        RegisterType rm;

        uint8_t pred;
        uint8_t succ;
        uint8_t fm;
    };

    union {
        uint64_t imm = 0;
        uint32_t shamt;
        uint32_t shamtw;
        uint32_t aqrl;
    };

    // Initialize instruction to be invalid so every return will
    // be either valid instruction or invalid one
    InstructionType type = InstructionType::INSTRUCTION_INVALID;
};

}  // namespace RISCV

#endif  // INCLUDE_DECODE_INSCTRUCTION_H
