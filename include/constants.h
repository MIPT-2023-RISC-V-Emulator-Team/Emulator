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
static constexpr uint64_t PHYS_MEMORY_BYTESIZE = 1ULL << 33;  // 8 GiB
static constexpr uint64_t VIRT_MEMORY_BYTESIZE = 1ULL << 33;  // 8 GiB
static constexpr uint64_t PHYS_PAGE_COUNT = PHYS_MEMORY_BYTESIZE / PAGE_BYTESIZE;

static constexpr uint32_t ADDRESS_PAGE_NUM_SHIFT = 12;
static constexpr uint32_t ADDRESS_PAGE_OFFSET_MASK = 0xFFF;

static constexpr uint32_t STACK_BYTESIZE = 1 << 20;  // 1 MiB
static constexpr uint64_t DEFAULT_STACK_ADDRESS = 0x3FFFFC00;

} // namespace memory

static constexpr uint32_t CSR_COUNT = 4096;
static constexpr uint32_t CSR_SATP_INDEX = 0x180;

// Sv48 mode
static constexpr uint64_t CSR_SATP_MODE_SV48 = 9;
static constexpr uint64_t CSR_SATP_MODE_BARE = 0;
static constexpr uint8_t  PTE_LEVELS_SV48 = 4;
static constexpr uint8_t  PTE_SIZE = 8;


enum RegisterType : uint8_t {
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

    ZERO = X0,
    RA = X1,
    SP = X2
};

}  // namespace RISCV

#endif  // INCLUDE_CONSTANTS_H
