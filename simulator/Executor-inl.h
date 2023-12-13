#ifndef EXECUTOR_INL_H
#define EXECUTOR_INL_H

#include <unistd.h>

#include "simulator/DecodedInstruction.h"
#include "simulator/Hart.h"
#include "simulator/memory/Memory.h"
#include "utils/macros.h"

namespace RISCV {

// ================================ PC ================================= //

static ALWAYS_INLINE void ExecutorLUI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lui     x%d, %ld\n", instr.rd, instr.imm);

    hart->setReg(instr.rd, instr.imm);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorAUIPC(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("auipc   x%d, %ld\n", instr.rd, instr.imm);

    hart->setReg(instr.rd, hart->getPC() + instr.imm);
    hart->incrementPC();
}

// =============================== Jumps =============================== //

static ALWAYS_INLINE void ExecutorJAL(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("jal     x%d, %ld\n", instr.rd, instr.imm);

    hart->setReg(instr.rd, hart->getPC() + INSTRUCTION_BYTESIZE);

    const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
    hart->setPC(nextPC);
}

static ALWAYS_INLINE void ExecutorJALR(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("jalr    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    const memory::VirtAddr returnPC = hart->getPC() + INSTRUCTION_BYTESIZE;
    const memory::VirtAddr nextPC = (hart->getReg(instr.rs1) + instr.imm) & ~1;

    hart->setReg(instr.rd, returnPC);
    hart->setPC(nextPC);
}

// ============================= Branching ============================= //

static ALWAYS_INLINE void ExecutorBEQ(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("beq     x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs == rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorBNE(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("bne     x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs != rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorBLT(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("blt     x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs < rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorBGE(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("bge     x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs >= rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorBLTU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("bltu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    const RegValue lhs = hart->getReg(instr.rs1);
    const RegValue rhs = hart->getReg(instr.rs2);

    if (lhs < rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorBGEU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("bgeu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    const RegValue lhs = hart->getReg(instr.rs1);
    const RegValue rhs = hart->getReg(instr.rs2);

    if (lhs >= rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

// =============================== Load ================================ //

static ALWAYS_INLINE void ExecutorLB(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lb      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint8_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, sext<7>(loaded));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorLH(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lh      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint16_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(loaded))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, sext<15>(loaded));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorLW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lw      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint32_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(loaded))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, sext<31>(loaded));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorLD(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("ld      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint64_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(loaded))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorLBU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lbu     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint8_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorLHU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lhu     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint16_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(loaded))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorLWU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("lwu     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint32_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(loaded))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.read(paddr, sizeof(loaded), &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

// =============================== Store =============================== //

static ALWAYS_INLINE void ExecutorSB(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sb      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint8_t stored = hart->getReg(instr.rs2);

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::WMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.write(paddr, sizeof(stored), &stored);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSH(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sh      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint16_t stored = hart->getReg(instr.rs2);

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(stored))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::WMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.write(paddr, sizeof(stored), &stored);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sw      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint32_t stored = hart->getReg(instr.rs2);

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(stored))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::WMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.write(paddr, sizeof(stored), &stored);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSD(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sd      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint64_t stored = hart->getReg(instr.rs2);

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;

    if (UNLIKELY(vaddr % sizeof(stored))) {
        std::cerr << "Error: unaligned memory access" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::WMem>(vaddr);

    memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
    pmem.write(paddr, sizeof(stored), &stored);

    hart->incrementPC();
}

// ======================= Arithmetic immediate ======================== //

static ALWAYS_INLINE void ExecutorADDI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("addi    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint64_t sum = hart->getReg(instr.rs1) + instr.imm;
    hart->setReg(instr.rd, sum);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLLI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("slli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    uint64_t result = hart->getReg(instr.rs1) << instr.shamt;
    hart->setReg(instr.rd, result);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLTI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("slti    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs = instr.imm;

    SignedRegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLTIU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sltiu   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue lhs = hart->getReg(instr.rs1);
    RegValue rhs = instr.imm;
    RegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorXORI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("xori    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue res = hart->getReg(instr.rs1) ^ instr.imm;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRLI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("srli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    RegValue res = hart->getReg(instr.rs1) >> (instr.shamt);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRAI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("srai    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    SignedRegValue res = static_cast<SignedRegValue>(hart->getReg(instr.rs1) >> (instr.imm));
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorORI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("ori     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue res = hart->getReg(instr.rs1) | instr.imm;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorANDI(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("andi    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue res = hart->getReg(instr.rs1) & instr.imm;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorADDIW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("addiw   x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue sum = hart->getReg(instr.rs1) + instr.imm;
    uint32_t sum32 = sum;

    hart->setReg(instr.rd, sext<31>(sum32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLLIW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("slliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    RegValue res = hart->getReg(instr.rs1) << instr.shamt;
    uint32_t res32 = res;

    hart->setReg(instr.rd, sext<31>(res32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRLIW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("srliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    RegValue res = hart->getReg(instr.rs1) >> instr.shamt;
    uint32_t res32 = res;

    hart->setReg(instr.rd, sext<31>(res32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRAIW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sraiw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    int32_t rs1_signed32 = hart->getReg(instr.rs1);
    int32_t res32 = static_cast<int32_t>(rs1_signed32 >> instr.imm);

    hart->setReg(instr.rd, sext<31>(res32));
    hart->incrementPC();
}

// ============================ Arithmetic ============================= //

static ALWAYS_INLINE void ExecutorADD(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("add     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->setReg(instr.rd, hart->getReg(instr.rs1) + hart->getReg(instr.rs2));

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLL(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sll     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) << hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLT(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("slt     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs = hart->getReg(instr.rs2);
    SignedRegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLTU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sltu    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue lhs = hart->getReg(instr.rs1);
    RegValue rhs = hart->getReg(instr.rs2);
    RegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorXOR(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("xor     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) ^ hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRL(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("srl     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) >> hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorOR(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("or      x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) | hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorAND(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("and     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) & hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSUB(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sub     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) - hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRA(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sra     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs = hart->getReg(instr.rs2);

    SignedRegValue res = 0;

    if (lhs < 0 && rhs > 0)
        res = lhs >> rhs | ~(~0U >> rhs);
    else
        res = lhs >> rhs;

    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorADDW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("addw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue sum = hart->getReg(instr.rs1) + hart->getReg(instr.rs2);
    uint32_t sum32 = sum;
    hart->setReg(instr.rd, sext<31>(sum32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSUBW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("subw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue diff = hart->getReg(instr.rs1) - hart->getReg(instr.rs2);
    uint32_t diff32 = diff;
    hart->setReg(instr.rd, sext<31>(diff32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSLLW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sllw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) << hart->getReg(instr.rs2);
    uint32_t res32 = res;
    hart->setReg(instr.rd, sext<31>(res32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRLW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("srlw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) >> hart->getReg(instr.rs2);
    uint32_t res32 = res;
    hart->setReg(instr.rd, sext<31>(res32));
    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorSRAW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("sraw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs = hart->getReg(instr.rs2);

    SignedRegValue res = 0;

    if (lhs < 0 && rhs > 0)
        res = lhs >> rhs | ~(~0U >> rhs);
    else
        res = lhs >> rhs;

    uint32_t res32 = res;
    hart->setReg(instr.rd, sext<31>(res32));
    hart->incrementPC();
}

// ========================== Miscellaneous ============================ //

static ALWAYS_INLINE void ExecutorFENCE(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("fence\n");

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorECALL(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("ecall   ");

    const SyscallRV syscallNo = static_cast<SyscallRV>(hart->getReg(RegisterType::A7));

    switch (syscallNo) {
        case SyscallRV::READ: {
            DEBUG_INSTRUCTION("SYS_read\n");

            uint64_t fileno = hart->getReg(RegisterType::A0);
            uint64_t vaddr = hart->getReg(RegisterType::A1);
            uint64_t length = hart->getReg(RegisterType::A2);

            char outStr[1024];
            const ssize_t retVal = read(fileno, outStr, length);
            // Return number of read bytes or error
            hart->setReg(RegisterType::A0, retVal);

            if (retVal > 0) {
                memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::WMem>(vaddr);
                memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
                pmem.write(paddr, retVal, outStr);
            }
            break;
        }
        case SyscallRV::WRITE: {
            DEBUG_INSTRUCTION("SYS_write\n");

            uint64_t fileno = hart->getReg(RegisterType::A0);
            uint64_t vaddr = hart->getReg(RegisterType::A1);
            uint64_t length = hart->getReg(RegisterType::A2);

            memory::PhysAddr paddr = hart->getPhysAddr<memory::MemoryType::RMem>(vaddr);

            char outStr[1024];

            memory::PhysicalMemory &pmem = memory::getPhysicalMemory();
            pmem.read(paddr, length, outStr);

            const ssize_t retVal = write(fileno, outStr, length);

            // Return number of written bytes or error
            hart->setReg(RegisterType::A0, retVal);
            break;
        }
        case SyscallRV::EXIT: {
            DEBUG_INSTRUCTION("SYS_exit\n");

            // By specification return value is in a0, but exit status
            // passed in the same register, so just set PC to zero
            hart->setPC(0);
            return;
        }

        case SyscallRV::IO_SETUP:
        case SyscallRV::IO_DESTROY:
        case SyscallRV::IO_SUBMIT:
        case SyscallRV::IO_CANCEL:
        case SyscallRV::IO_GETEVENTS:
        case SyscallRV::SETXATTR:
        case SyscallRV::LSETXATTR:
        case SyscallRV::FSETXATTR:
        case SyscallRV::GETXATTR:
        case SyscallRV::LGETXATTR:
        case SyscallRV::FGETXATTR:
        case SyscallRV::LISTXATTR:
        case SyscallRV::LLISTXATTR:
        case SyscallRV::FLISTXATTR:
        case SyscallRV::REMOVEXATTR:
        case SyscallRV::LREMOVEXATTR:
        case SyscallRV::FREMOVEXATTR:
        case SyscallRV::GETCWD:
        case SyscallRV::LOOKUP_DCOOCKIE:
        case SyscallRV::EVENTFD2:
        case SyscallRV::EPOLL_CREATE1:
        case SyscallRV::EPOLL_CTL:
        case SyscallRV::EPOLL_PWAIT:
        case SyscallRV::DUP:
        case SyscallRV::DUP3:
        case SyscallRV::FCNTL64:
        case SyscallRV::INOTIFY_INIT1:
        case SyscallRV::INOTIFY_ADD_WATCH:
        case SyscallRV::INOTIFY_RM_WATCH:
        case SyscallRV::IOCTL:
        case SyscallRV::IOPRIO_SET:
        case SyscallRV::IOPRIO_GET:
        case SyscallRV::FLOCK:
        case SyscallRV::MKNODAT:
        case SyscallRV::MKDIRAT:
        case SyscallRV::UNLINKAT:
        case SyscallRV::SYMLINKAT:
        case SyscallRV::LINKAT:
        case SyscallRV::RENAMEAT:
        case SyscallRV::UMOUNT:
        case SyscallRV::MOUNT:
        case SyscallRV::PIVOT_ROOT:
        case SyscallRV::NI_SYSCALL:
        case SyscallRV::STATFS64:
        case SyscallRV::FSTATFS64:
        case SyscallRV::TRUNCATE64:
        case SyscallRV::FTRUNCATE64:
        case SyscallRV::FALLOCATE:
        case SyscallRV::FACCESSAT:
        case SyscallRV::CHDIR:
        case SyscallRV::FCHDIR:
        case SyscallRV::CHROOT:
        case SyscallRV::FCHMOD:
        case SyscallRV::FCHMODAT:
        case SyscallRV::FCHOWNAT:
        case SyscallRV::FCHOWN:
        case SyscallRV::OPENAT:
        case SyscallRV::CLOSE:
        case SyscallRV::VHANGUP:
        case SyscallRV::PIPE2:
        case SyscallRV::QUOTACTL:
        case SyscallRV::GETDENTS64:
        case SyscallRV::LSEEK:
        case SyscallRV::READV:
        case SyscallRV::WRITEV:
        case SyscallRV::PREAD64:
        case SyscallRV::PWRITE64:
        case SyscallRV::PREADV:
        case SyscallRV::PWRITEV:
        case SyscallRV::SENDFILE64:
        case SyscallRV::PSELECT6_TIME32:
        case SyscallRV::PPOLL_TIME32:
        case SyscallRV::SIGNALFD4:
        case SyscallRV::VMSPLICE:
        case SyscallRV::SPLICE:
        case SyscallRV::TEE:
        case SyscallRV::READLINKAT:
        case SyscallRV::NEWFSTATAT:
        case SyscallRV::NEWFSTAT:
        case SyscallRV::SYNC:
        case SyscallRV::FSYNC:
        case SyscallRV::FDATASYNC:
        case SyscallRV::SYNC_FILE_RANGE:
        case SyscallRV::TIMERFD_CREATE:
        case SyscallRV::TIMERFD_SETTIME:
        case SyscallRV::TIMERFD_GETTIME:
        case SyscallRV::UTIMENSAT:
        case SyscallRV::ACCT:
        case SyscallRV::CAPGET:
        case SyscallRV::CAPSET:
        case SyscallRV::PERSONALITY:
        case SyscallRV::EXIT_GROUP:
        case SyscallRV::WAITID:
        case SyscallRV::SET_TID_ADDRESS:
        case SyscallRV::UNSHARE:
        case SyscallRV::FUTEX:
        case SyscallRV::SET_ROBUST_LIST:
        case SyscallRV::GET_ROBUST_LIST:
        case SyscallRV::NANOSLEEP:
        case SyscallRV::GETITIMER:
        case SyscallRV::SETITIMER:
        case SyscallRV::KEXEC_LOAD:
        case SyscallRV::INIT_MODULE:
        case SyscallRV::DELETE_MODULE:
        case SyscallRV::TIMER_CREATE:
        case SyscallRV::TIMER_GETTIME:
        case SyscallRV::TIMER_GETOVERRUN:
        case SyscallRV::TIMER_SETTIME:
        case SyscallRV::TIMER_DELETE:
        case SyscallRV::CLOCK_SETTIME:
        case SyscallRV::CLOCK_GETTIME:
        case SyscallRV::CLOCK_GETRES:
        case SyscallRV::CLOCK_NANOSLEEP:
        case SyscallRV::SYSLOG:
        case SyscallRV::PTRACE:
        case SyscallRV::SCHED_SETPARAM:
        case SyscallRV::SCHED_SETSCHEDULER:
        case SyscallRV::SCHED_GETSCHEDULER:
        case SyscallRV::SCHED_GETPARAM:
        case SyscallRV::SCHED_SETAFFINITY:
        case SyscallRV::SCHED_GETAFFINITY:
        case SyscallRV::SCHED_YIELD:
        case SyscallRV::SCHED_GET_PRIORITY_MAX:
        case SyscallRV::SCHED_GET_PRIORITY_MIN:
        case SyscallRV::SCHED_RR_GET_INTERVAL:
        case SyscallRV::RESTART_SYSCALL:
        case SyscallRV::KILL:
        case SyscallRV::TKILL:
        case SyscallRV::TGKILL:
        case SyscallRV::SIGALTSTACK:
        case SyscallRV::RT_SIGSUSPEND:
        case SyscallRV::RT_SIGACTION:
        case SyscallRV::RT_SIGPROCMASK:
        case SyscallRV::RT_SIGPENDING:
        case SyscallRV::RT_SIGTIMEDWAIT_TIME32:
        case SyscallRV::RT_SIGQUEUEINFO:
        case SyscallRV::SETPRIORITY:
        case SyscallRV::GETPRIORITY:
        case SyscallRV::REBOOT:
        case SyscallRV::SETREGID:
        case SyscallRV::SETGID:
        case SyscallRV::SETREUID:
        case SyscallRV::SETUID:
        case SyscallRV::SETRESUID:
        case SyscallRV::GETRESUID:
        case SyscallRV::SETRESGID:
        case SyscallRV::GETRESGID:
        case SyscallRV::SETFSUID:
        case SyscallRV::SETFSGID:
        case SyscallRV::TIMES:
        case SyscallRV::SETPGID:
        case SyscallRV::GETPGID:
        case SyscallRV::GETSID:
        case SyscallRV::SETSID:
        case SyscallRV::GETGROUPS:
        case SyscallRV::SETGROUPS:
        case SyscallRV::NEWUNAME:
        case SyscallRV::SETHOSTNAME:
        case SyscallRV::SETDOMAINNAME:
        case SyscallRV::GETRLIMIT:
        case SyscallRV::SETRLIMIT:
        case SyscallRV::GETRUSAGE:
        case SyscallRV::UMASK:
        case SyscallRV::PRCTL:
        case SyscallRV::GETCPU:
        case SyscallRV::GETTIMEOFDAY:
        case SyscallRV::SETTIMEOFDAY:
        case SyscallRV::ADJTIMEX:
        case SyscallRV::GETPID:
        case SyscallRV::GETPPID:
        case SyscallRV::GETUID:
        case SyscallRV::GETEUID:
        case SyscallRV::GETGID:
        case SyscallRV::GETEGID:
        case SyscallRV::GETTID:
        case SyscallRV::SYSINFO:
        case SyscallRV::MQ_OPEN:
        case SyscallRV::MQ_UNLINK:
        case SyscallRV::MQ_TIMEDSEND:
        case SyscallRV::MQ_TIMEDRECEIVE:
        case SyscallRV::MQ_NOTIFY:
        case SyscallRV::MQ_GETSETATTR:
        case SyscallRV::MSGGET:
        case SyscallRV::MSGCTL:
        case SyscallRV::MSGRCV:
        case SyscallRV::MSGSND:
        case SyscallRV::SEMGET:
        case SyscallRV::SEMCTL:
        case SyscallRV::SEMTIMEDOP:
        case SyscallRV::SEMOP:
        case SyscallRV::SHMGET:
        case SyscallRV::SHMCTL:
        case SyscallRV::SHMAT:
        case SyscallRV::SHMDT:
        case SyscallRV::SOCKET:
        case SyscallRV::SOCKETPAIR:
        case SyscallRV::BIND:
        case SyscallRV::LISTEN:
        case SyscallRV::ACCEPT:
        case SyscallRV::CONNECT:
        case SyscallRV::GETSOCKNAME:
        case SyscallRV::GETPEERNAME:
        case SyscallRV::SENDTO:
        case SyscallRV::RECVFROM:
        case SyscallRV::SETSOCKOPT:
        case SyscallRV::GETSOCKOPT:
        case SyscallRV::SHUTDOWN:
        case SyscallRV::SENDMSG:
        case SyscallRV::RECVMSG:
        case SyscallRV::READAHEAD:
        case SyscallRV::BRK:
        case SyscallRV::MUNMAP:
        case SyscallRV::MREMAP:
        case SyscallRV::ADD_KEY:
        case SyscallRV::REQUEST_KEY:
        case SyscallRV::KEYCTL:
        case SyscallRV::CLONE:
        case SyscallRV::EXECVE:
        case SyscallRV::MMAP:
        case SyscallRV::FADVISE64_64:
        case SyscallRV::SWAPON:
        case SyscallRV::SWAPOFF:
        case SyscallRV::MPROTECT:
        case SyscallRV::MSYNC:
        case SyscallRV::MLOCK:
        case SyscallRV::MUNLOCK:
        case SyscallRV::MLOCKALL:
        case SyscallRV::MUNLOCKALL:
        case SyscallRV::MINCORE:
        case SyscallRV::MADVISE:
        case SyscallRV::REMAP_FILE_PAGES:
        case SyscallRV::MBIND:
        case SyscallRV::GET_MEMPOLICY:
        case SyscallRV::SET_MEMPOLICY:
        case SyscallRV::MIGRATE_PAGES:
        case SyscallRV::MOVE_PAGES:
        case SyscallRV::RT_TGSIGQUEUEINFO:
        case SyscallRV::PERF_EVENT_OPEN:
        case SyscallRV::ACCEPT4:
        case SyscallRV::RECVMMSG_TIME32:
        case SyscallRV::WAIT4:
        case SyscallRV::PRLIMIT64:
        case SyscallRV::FANOTIFY_INIT:
        case SyscallRV::FANOTIFY_MARK:
        case SyscallRV::NAME_TO_HANDLE_AT:
        case SyscallRV::OPEN_BY_HANDLE_AT:
        case SyscallRV::CLOCK_ADJTIME:
        case SyscallRV::SYNCFS:
        case SyscallRV::SETNS:
        case SyscallRV::SENDMMSG:
        case SyscallRV::PROCESS_VM_READV:
        case SyscallRV::PROCESS_VM_WRITEV:
        case SyscallRV::KCMP:
        case SyscallRV::FINIT_MODULE:
        case SyscallRV::SCHED_SETATTR:
        case SyscallRV::SCHED_GETATTR:
        case SyscallRV::RENAMEAT2:
        case SyscallRV::SECCOMP:
        case SyscallRV::GETRANDOM:
        case SyscallRV::MEMFD_CREATE:
        case SyscallRV::BPF:
        case SyscallRV::EXECVEAT:
        case SyscallRV::USERFAULTFD:
        case SyscallRV::MEMBARRIER:
        case SyscallRV::MLOCK2:
        case SyscallRV::COPY_FILE_RANGE:
        case SyscallRV::PREADV2:
        case SyscallRV::PWRITEV2:
        case SyscallRV::PKEY_MPROTECT:
        case SyscallRV::PKEY_ALLOC:
        case SyscallRV::PKEY_FREE:
        case SyscallRV::STATX:
        case SyscallRV::IO_PGETEVENTS:
        case SyscallRV::RSEQ:
        case SyscallRV::KEXEC_FILE_LOAD:
        case SyscallRV::PIDFD_SEND_SIGNAL:
        case SyscallRV::IO_URING_SETUP:
        case SyscallRV::IO_URING_ENTER:
        case SyscallRV::IO_URING_REGISTER:
        case SyscallRV::OPEN_TREE:
        case SyscallRV::MOVE_MOUNT:
        case SyscallRV::FSOPEN:
        case SyscallRV::FSCONFIG:
        case SyscallRV::FSMOUNT:
        case SyscallRV::FSPICK:
        case SyscallRV::PIDFD_OPEN:
        case SyscallRV::CLONE3:
        case SyscallRV::CLOSE_RANGE:
        case SyscallRV::OPENAT2:
        case SyscallRV::PIDFD_GETFD:
        case SyscallRV::FACCESSAT2:
        case SyscallRV::PROCESS_MADVISE: {
            std::cerr << "unimplemented syscall was called" << std::endl;
            break;
        }

        default: {
            std::cerr << "unknown syscall" << std::endl;
            break;
        }
    }

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorEBREAK(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("ebreak\n");

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorFENCEI(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorMUL(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("mul     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t rs1_l = hart->getReg(instr.rs1) & 0xFFFFFFFF;
    uint64_t rs2_l = hart->getReg(instr.rs2) & 0xFFFFFFFF;

    uint64_t prod = rs1_l * rs2_l;
    hart->setReg(instr.rd, prod);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorMULH(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("mulh    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t rs1_l = hart->getReg(instr.rs1) & 0xFFFFFFFF;
    int64_t rs1_h = (hart->getReg(instr.rs1) & ~0xFFFFFFFF) >> 32;
    uint64_t rs2_l = hart->getReg(instr.rs2) & 0xFFFFFFFF;
    int64_t rs2_h = (hart->getReg(instr.rs2) & ~0xFFFFFFFF) >> 32;

    int64_t prod = (rs1_h * rs2_h + (rs1_h * rs2_l + rs1_l * rs2_h)) >> 32;
    hart->setReg(instr.rd, prod);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorMULHSU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("mulhsu  x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t rs1_l = hart->getReg(instr.rs1) & 0xFFFFFFFF;
    int64_t rs1_h = (hart->getReg(instr.rs1) & ~0xFFFFFFFF) >> 32;
    uint64_t rs2_l = hart->getReg(instr.rs2) & 0xFFFFFFFF;
    uint64_t rs2_h = (hart->getReg(instr.rs2) & ~0xFFFFFFFF) >> 32;

    int64_t prod = (rs1_h * rs2_h + (rs1_h * rs2_l + rs1_l * rs2_h)) >> 32;
    hart->setReg(instr.rd, prod);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorMULHU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("mulhu   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t rs1_l = hart->getReg(instr.rs1) & 0xFFFFFFFF;
    uint64_t rs1_h = (hart->getReg(instr.rs1) & ~0xFFFFFFFF) >> 32;
    uint64_t rs2_l = hart->getReg(instr.rs2) & 0xFFFFFFFF;
    uint64_t rs2_h = (hart->getReg(instr.rs2) & ~0xFFFFFFFF) >> 32;

    uint64_t prod = (rs1_h * rs2_h + (rs1_h * rs2_l + rs1_l * rs2_h)) >> 32;
    hart->setReg(instr.rd, prod);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorDIV(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("div     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int64_t dividend = hart->getReg(instr.rs1);
    int64_t divisor = hart->getReg(instr.rs2);

    int64_t quot = dividend / divisor;
    hart->setReg(instr.rd, quot);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorDIVU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("divu    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t dividend = hart->getReg(instr.rs1);
    uint64_t divisor = hart->getReg(instr.rs2);

    uint64_t quot = dividend / divisor;
    hart->setReg(instr.rd, quot);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorREM(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("rem     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int64_t dividend = hart->getReg(instr.rs1);
    int64_t divisor = hart->getReg(instr.rs2);

    int64_t rem = dividend % divisor;
    hart->setReg(instr.rd, rem);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorREMU(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("remu    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t dividend = hart->getReg(instr.rs1);
    uint64_t divisor = hart->getReg(instr.rs2);

    uint64_t rem = dividend % divisor;
    hart->setReg(instr.rd, rem);

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorMULW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("mulw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t rs1_l = hart->getReg(instr.rs1) & 0xFFFFFFFF;
    uint64_t rs2_l = hart->getReg(instr.rs2) & 0xFFFFFFFF;

    uint32_t prod = rs1_l * rs2_l;
    hart->setReg(instr.rd, sext<31>(prod));

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorDIVW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("divw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int32_t dividend32 = hart->getReg(instr.rs1);
    int32_t divisor32 = hart->getReg(instr.rs2);

    int32_t quot32 = dividend32 / divisor32;
    hart->setReg(instr.rd, sext<31>(quot32));

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorDIVUW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("divuw   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint32_t dividend32 = hart->getReg(instr.rs1);
    uint32_t divisor32 = hart->getReg(instr.rs2);

    uint32_t quot32 = dividend32 / divisor32;
    hart->setReg(instr.rd, sext<31>(quot32));

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorREMW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("remw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int32_t dividend32 = hart->getReg(instr.rs1);
    int32_t divisor32 = hart->getReg(instr.rs2);

    int32_t rem32 = dividend32 % divisor32;
    hart->setReg(instr.rd, sext<31>(rem32));

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorREMUW(Hart *hart, const DecodedInstruction &instr) {
    DEBUG_INSTRUCTION("remuw   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint32_t dividend32 = hart->getReg(instr.rs1);
    uint32_t divisor32 = hart->getReg(instr.rs2);

    uint32_t rem32 = dividend32 % divisor32;
    hart->setReg(instr.rd, sext<31>(rem32));

    hart->incrementPC();
}

static ALWAYS_INLINE void ExecutorAMOADDW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOXORW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOORW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOANDW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMINW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMAXW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMINUW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMAXUW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOSWAPW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorLRW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorSCW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOADDD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOXORD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOORD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOANDD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMIND(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMAXD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMINUD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOMAXUD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorAMOSWAPD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorLRD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorSCD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorURET(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorSRET(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorMRET(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorDRET(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorSFENCEVMA(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorWFI(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorCSRRW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorCSRRS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorCSRRC(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorCSRRWI(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorCSRRSI(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorCSRRCI(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorHFENCEVVMA(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorHFENCEGVMA(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFADDS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSUBS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMULS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFDIVS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJNS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJXS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMINS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMAXS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSQRTS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFADDD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSUBD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMULD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFDIVD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJND(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJXD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMIND(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMAXD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTSD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTDS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSQRTD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFADDQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSUBQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMULQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFDIVQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJNQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSGNJXQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMINQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMAXQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTSQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTQS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTDQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTQD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSQRTQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLES(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLTS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFEQS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLED(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLTD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFEQD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLEQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLTQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFEQQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTWS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTWUS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTLS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTLUS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMVXW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCLASSS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTWD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTWUD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTLD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTLUD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMVXD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCLASSD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTWQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTWUQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTLQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTLUQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMVXQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCLASSQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTSW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTSWU(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTSL(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTSLU(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMVWX(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTDW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTDWU(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTDL(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTDLU(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMVDX(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTQW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTQWU(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTQL(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFCVTQLU(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMVQX(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFLQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSW(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFSQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMADDS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMSUBS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFNMSUBS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFNMADDS(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMADDD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMSUBD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFNMSUBD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFNMADDD(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMADDQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFMSUBQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFNMSUBQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

static ALWAYS_INLINE void ExecutorFNMADDQ(Hart *hart, const DecodedInstruction &instr) {
    UNREACHABLE();
}

}  // namespace RISCV

#endif  // EXECUTOR_INL_H
