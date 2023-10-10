#ifndef DECODER_H
#define DECODER_H

#include "Common.h"

namespace RISCV {

template <int high, int low>
static inline uint32_t getPartialBits(const EncodedInstruction val) {
    constexpr const uint32_t mask = ((1 << (high - low + 1)) - 1) << low;
    return (val & mask);
}

template <int high, int low>
static inline uint32_t getPartialBitsShifted(const EncodedInstruction val) {
    constexpr const uint32_t mask = ((1 << (high - low + 1)) - 1) << low;
    return (val & mask) >> low;
}

class Decoder {
public:
    void decodeInstruction(const EncodedInstruction encInstr, DecodedInstruction& decInstr) const;
};

}  // namespace RISCV

#endif  // DECODER_H
