#ifndef DECODER_H
#define DECODER_H

#include "Common.h"

namespace RISCV {

class Decoder {
public:
    DecodedInstruction decodeInstruction(const EncodedInstruction encInstr) const;
};

template <int8_t shift>
inline constexpr uint32_t shiftRight(uint32_t val) {
    return val >> shift;
}

template <int8_t shift>
inline constexpr uint32_t shiftLeft(uint32_t val) {
    return val << shift;
}

template <uint8_t low, uint8_t high>
inline constexpr uint32_t getPartialBits(uint32_t val) {
    static_assert(low <= high);
    constexpr uint32_t mask = ((1 << (high - low + 1)) - 1) << low;
    return val & mask;
}

template <uint32_t opcode, uint32_t mask>
inline constexpr uint32_t getOpcodeBits() {
    return opcode & mask;
}

}  // namespace RISCV

#endif  // DECODER_H
