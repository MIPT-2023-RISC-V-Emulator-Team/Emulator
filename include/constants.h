#ifndef INCLUDE_CONSTANTS_H
#define INCLUDE_CONSTANTS_H

#include <array>
#include <cstdint>
#include <string_view>

namespace RISCV {

static constexpr uint8_t INSTRUCTION_BYTESIZE = 4;

namespace memory {

static constexpr uint32_t PAGE_BYTESIZE = 1 << 12;            // 4 KiB
static constexpr uint64_t PHYS_MEMORY_BYTESIZE = 1ULL << 33;  // 8 GiB
static constexpr uint64_t VIRT_MEMORY_BYTESIZE = 1ULL << 33;  // 8 GiB
static constexpr uint32_t PHYS_PAGE_COUNT = PHYS_MEMORY_BYTESIZE / PAGE_BYTESIZE;

static constexpr uint32_t ADDRESS_PAGE_NUM_SHIFT = 12;
static constexpr uint32_t ADDRESS_PAGE_OFFSET_MASK = 0xFFF;

static constexpr uint32_t STACK_BYTESIZE = 1 << 20;  // 1 MiB
static constexpr uint64_t DEFAULT_STACK_ADDRESS = VIRT_MEMORY_BYTESIZE - STACK_BYTESIZE;

}  // namespace memory

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

enum InstructionType : uint16_t {
  LUI = 0,
  AUIPC,
  JAL,
  JALR,
  BEQ,
  BNE,
  BLT,
  BGE,
  BLTU,
  BGEU,
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
  ADDI,
  SLTI,
  SLTIU,
  XORI,
  ORI,
  ANDI,
  SLLI,
  SRLI,
  SRAI,
  ADD,
  SUB,
  SLL,
  SLT,
  SLTU,
  XOR,
  SRL,
  SRA,
  OR,
  AND,
  FENCE,
  ECALL,
  EBREAK,
  LWU,
  LD,
  SD,
  ADDIW,
  SLLIW,
  SRLIW,
  SRAIW,
  ADDW,
  SUBW,
  SLLW,
  SRLW,
  SRAW,

  INSTRUCTION_COUNT,

  INSTRUCTION_INVALID
};

static constexpr std::array<std::string_view, InstructionType::INSTRUCTION_COUNT> InstructionNames =
    {"lui",   "auipc", "jal",   "jalr",   "beq",  "bne",  "blt",  "bge",   "bltu",
     "bgeu",  "lb",    "lh",    "lw",     "lbu",  "lhu",  "sb",   "sh",    "sw",
     "addi",  "slti",  "sltiu", "xori",   "ori",  "andi", "slli", "srli",  "srai",
     "add",   "sub",   "sll",   "slt",    "sltu", "xor",  "srl",  "sra",   "or",
     "and",   "fence", "ecall", "ebreak", "lwu",  "ld",   "sd",   "addiw", "slliw",
     "srliw", "sraiw", "addw",  "subw",   "sllw", "srlw", "sraw"};

}  // namespace RISCV

#endif  // INCLUDE_CONSTANTS_H
