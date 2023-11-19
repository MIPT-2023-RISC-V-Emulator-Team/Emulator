#ifndef INCLUDE_CONSTANTS_H
#define INCLUDE_CONSTANTS_H

#include <array>
#include <cstdint>
#include <string_view>

#include "generated/InstructionTypes.h"

namespace RISCV {

static constexpr uint8_t INSTRUCTION_BYTESIZE = 4;

namespace memory {

static constexpr uint32_t PAGE_BYTESIZE = 1 << 12;            // 4 KiB
static constexpr uint64_t PHYS_MEMORY_BYTESIZE = 1ULL << 30;  // 1 GiB
static constexpr uint64_t VIRT_MEMORY_BYTESIZE = 1ULL << 34;  // 16 GiB
static constexpr uint64_t PHYS_PAGE_COUNT = PHYS_MEMORY_BYTESIZE / PAGE_BYTESIZE;

static constexpr uint32_t ADDRESS_PAGE_NUM_SHIFT = 12;
static constexpr uint32_t ADDRESS_PAGE_OFFSET_MASK = 0xFFF;

static constexpr uint32_t STACK_BYTESIZE = 1 << 24;  // 16 MiB
static constexpr uint64_t DEFAULT_STACK_ADDRESS = 0x3FFFFC00;

}  // namespace memory

static constexpr uint32_t CSR_COUNT = 4096;
static constexpr uint32_t CSR_SATP_INDEX = 0x180;

// Sv48 mode
static constexpr uint8_t PTE_LEVELS_SV48 = 4;
static constexpr uint8_t PTE_SIZE = 8;

enum TranslationMode : uint64_t {
    TRANSLATION_MODE_BARE = 0,
    TRANSLATION_MODE_SV39 = 8,
    TRANSLATION_MODE_SV48 = 9,
    TRANSLATION_MODE_SV57 = 10,
    TRANSLATION_MODE_SV64 = 11
};

enum RegisterType : uint8_t {

    /*
     * Numeric names
     */

    X0 = 0,
    X1,
    X2,
    X3,
    X4,
    X5,
    X6,
    X7,
    X8,
    X9,
    X10,
    X11,
    X12,
    X13,
    X14,
    X15,
    X16,
    X17,
    X18,
    X19,
    X20,
    X21,
    X22,
    X23,
    X24,
    X25,
    X26,
    X27,
    X28,
    X29,
    X30,
    X31,

    REGISTER_COUNT,

    /*
     * ABI names
     */

    ZERO = X0,
    RA = X1,
    SP = X2,
    GP = X3,
    TP = X4,
    T0 = X5,
    T1 = X6,
    T2 = X7,
    FP = X8,
    S0 = X8,
    S1 = X9,
    A0 = X10,
    A1 = X11,
    A2 = X12,
    A3 = X13,
    A4 = X14,
    A5 = X15,
    A6 = X16,
    A7 = X17,
    S2 = X18,
    S3 = X19,
    S4 = X20,
    S5 = X21,
    S6 = X22,
    S7 = X23,
    S8 = X24,
    S9 = X25,
    S10 = X26,
    S11 = X27,
    T3 = X28,
    T4 = X29,
    T5 = X30,
    T6 = X31
};

}  // namespace RISCV

#endif  // INCLUDE_CONSTANTS_H
