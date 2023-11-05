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


template <int8_t shift, typename T = uint32_t>
inline constexpr T shiftRight(const T val) {
    return val >> shift;
}

template <int8_t shift, typename T = uint32_t>
inline constexpr T shiftLeft(const T val) {
    return val << shift;
}

template <uint8_t low, uint8_t high, typename T = uint32_t>
inline constexpr T getPartialBits(const T val) {
    static_assert(low <= high);
    constexpr T mask = ((static_cast<T>(1) << (high - low + 1)) - 1) << low;
    return val & mask;
}

template <uint8_t low, uint8_t high, typename T = uint32_t>
inline constexpr T getPartialBitsShifted(const T val) {
    static_assert(low <= high);
    constexpr T mask = ((static_cast<T>(1) << (high - low + 1)) - 1) << low;
    return shiftRight<low, T>(val & mask);
}

template<uint8_t low, uint8_t high, typename T = uint32_t>
inline constexpr T makePartialBits(const T val) {
    static_assert(low <= high);
    constexpr const T mask = ((static_cast<T>(1) << (high - low + 1)) - 1);
    return ((val & mask) << low);
}

template <uint32_t opcode, uint32_t mask>
inline constexpr uint32_t getOpcodeBits() {
    return opcode & mask;
}

}  // namespace RISCV

#endif  // COMMON_H
