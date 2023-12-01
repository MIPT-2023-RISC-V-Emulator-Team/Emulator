#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

namespace RISCV {

using RegValue = uint64_t;
using SignedRegValue = int64_t;

using EncodedInstruction = uint32_t;

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

template <uint8_t low, uint8_t high, typename T = uint32_t>
inline constexpr T makePartialBits(const T val) {
    static_assert(low <= high);
    constexpr const T mask = ((static_cast<T>(1) << (high - low + 1)) - 1);
    return ((val & mask) << low);
}

template <uint32_t opcode, uint32_t mask>
inline constexpr uint32_t getOpcodeBits() {
    return opcode & mask;
}

// Sign Extend. signBitNum = 0, 1, 2, ..., 31 from right to left
template<uint8_t signBitNum>
static inline uint64_t sext(const uint32_t val) {
    constexpr uint64_t upperBitsAllOnes = ~((1ULL << signBitNum + 1) - 1);
    constexpr uint32_t signBit = 1 << signBitNum;
    if (val & signBit) {
        return upperBitsAllOnes | val;
    }
    return val;
}

// Zero Extend
static inline uint64_t zext(const uint32_t val) {
    const uint64_t tmp = val;
    return tmp;
}

}  // namespace RISCV

#endif  // COMMON_H
