#include "DecodedInstruction.h"
#include "Hart.h"
#include "macros.h"

namespace RISCV {

// ================================ PC ================================= //

ALWAYS_INLINE void ExecutorLUI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lui     x%d, %ld\n", instr.rd, instr.imm);

    hart->setReg(instr.rd, instr.imm);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorAUIPC(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("auipc   x%d, %ld\n", instr.rd, instr.imm);

    hart->setReg(instr.rd, hart->getPC() + instr.imm);
    hart->incrementPC();
}

// =============================== Jumps =============================== //

ALWAYS_INLINE void ExecutorJAL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("jal     x%d, %ld\n", instr.rd, instr.imm);

    hart->setReg(instr.rd, hart->getPC() + INSTRUCTION_BYTESIZE);

    const memory::VirtAddr nextPC = hart->getPC() + instr.imm;
    hart->setPC(nextPC);
}

ALWAYS_INLINE void ExecutorJALR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("jalr    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    const memory::VirtAddr returnPC = hart->getPC() + INSTRUCTION_BYTESIZE;
    const memory::VirtAddr nextPC = (hart->getReg(instr.rs1) + instr.imm) & ~1;

    hart->setReg(instr.rd, returnPC);
    hart->setPC(nextPC);
}

// ============================= Branching ============================= //

ALWAYS_INLINE void ExecutorBEQ(Hart* hart, const DecodedInstruction& instr) {
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

ALWAYS_INLINE void ExecutorBNE(Hart* hart, const DecodedInstruction& instr) {
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

ALWAYS_INLINE void ExecutorBLT(Hart* hart, const DecodedInstruction& instr) {
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

ALWAYS_INLINE void ExecutorBGE(Hart* hart, const DecodedInstruction& instr) {
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

ALWAYS_INLINE void ExecutorBLTU(Hart* hart, const DecodedInstruction& instr) {
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

ALWAYS_INLINE void ExecutorBGEU(Hart* hart, const DecodedInstruction& instr) {
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

ALWAYS_INLINE void ExecutorLB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lb      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint8_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 1, &loaded);

    hart->setReg(instr.rd, sext(loaded, 7));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorLH(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lh      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint16_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 2, &loaded);

    hart->setReg(instr.rd, sext(loaded, 15));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lw      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint32_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 4, &loaded);

    hart->setReg(instr.rd, sext(loaded, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorLD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ld      x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint64_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 8, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorLBU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lbu     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint8_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 1, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorLHU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lhu     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint16_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 2, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorLWU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lwu     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint32_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 4, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

// =============================== Store =============================== //

ALWAYS_INLINE void ExecutorSB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sb      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint8_t stored = hart->getReg(instr.rs2) & 0xFF;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrW(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 1, &stored);

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSH(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sh      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint16_t stored = hart->getReg(instr.rs2) & 0xFFFF;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrW(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 2, &stored);

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sw      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint32_t stored = hart->getReg(instr.rs2) & 0xFFFFFFFF;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrW(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 4, &stored);

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sd      x%d, x%d, %ld\n", instr.rs1, instr.rs2, instr.imm);

    uint64_t stored = hart->getReg(instr.rs2);

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + instr.imm;
    memory::PhysAddr paddr = hart->getPhysAddrW(vaddr);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 8, &stored);

    hart->incrementPC();
}

// ======================= Arithmetic immediate ======================== //

ALWAYS_INLINE void ExecutorADDI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("addi    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    uint64_t sum = hart->getReg(instr.rs1) + instr.imm;
    hart->setReg(instr.rd, sum);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLLI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    uint64_t result = hart->getReg(instr.rs1) << instr.shamt;
    hart->setReg(instr.rd, result);

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLTI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slti    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs = instr.imm;

    SignedRegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLTIU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sltiu   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue lhs = hart->getReg(instr.rs1);
    RegValue rhs = instr.imm;
    RegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorXORI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("xori    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue res = hart->getReg(instr.rs1) ^ instr.imm ;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRLI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    RegValue res = hart->getReg(instr.rs1) >> (instr.shamt);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRAI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srai    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    SignedRegValue res = static_cast<SignedRegValue>(hart->getReg(instr.rs1) >> (instr.imm));
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorORI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ori     x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue res = hart->getReg(instr.rs1) | instr.imm;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorANDI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("andi    x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue res = hart->getReg(instr.rs1) & instr.imm;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorADDIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("addiw   x%d, x%d, %ld\n", instr.rd, instr.rs1, instr.imm);

    RegValue sum = hart->getReg(instr.rs1) + instr.imm;
    uint32_t sum32 = sum;

    hart->setReg(instr.rd, sext(sum32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLLIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    RegValue res = hart->getReg(instr.rs1) << instr.shamt;
    uint32_t res32 = res;

    hart->setReg(instr.rd, sext(res32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRLIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    RegValue res = hart->getReg(instr.rs1) >> instr.shamt;
    uint32_t res32 = res;

    hart->setReg(instr.rd, sext(res32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRAIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sraiw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    int32_t rs1_signed32 = hart->getReg(instr.rs1);
    int32_t res32 = static_cast<int32_t>(rs1_signed32 >> instr.imm);

    hart->setReg(instr.rd, sext(res32, 31));
    hart->incrementPC();
}

// ============================ Arithmetic ============================= //

ALWAYS_INLINE void ExecutorADD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("add     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->setReg(instr.rd, hart->getReg(instr.rs1) + hart->getReg(instr.rs2));

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sll     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) << hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLT(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slt     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs = hart->getReg(instr.rs2);
    SignedRegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLTU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sltu    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue lhs = hart->getReg(instr.rs1);
    RegValue rhs = hart->getReg(instr.rs2);
    RegValue res = lhs < rhs;
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorXOR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("xor     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) ^ hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srl     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) >> hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorOR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("or      x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) | hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorAND(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("and     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) & hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSUB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sub     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) - hart->getReg(instr.rs2);
    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRA(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sra     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs =  hart->getReg(instr.rs2);

    SignedRegValue res = 0;

    if (lhs < 0 && rhs > 0)
        res = lhs >> rhs | ~(~0U >> rhs);
    else
        res = lhs >> rhs;

    hart->setReg(instr.rd, res);
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorADDW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("addw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue sum = hart->getReg(instr.rs1) + hart->getReg(instr.rs2);
    uint32_t sum32 = sum;
    hart->setReg(instr.rd, sext(sum32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSUBW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("subw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue diff = hart->getReg(instr.rs1) - hart->getReg(instr.rs2);
    uint32_t diff32 = diff;
    hart->setReg(instr.rd, sext(diff32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSLLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sllw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) << hart->getReg(instr.rs2);
    uint32_t res32 = res;
    hart->setReg(instr.rd, sext(res32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srlw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    RegValue res = hart->getReg(instr.rs1) >> hart->getReg(instr.rs2);
    uint32_t res32 = res;
    hart->setReg(instr.rd, sext(res32, 31));
    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorSRAW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sraw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    SignedRegValue lhs = hart->getReg(instr.rs1);
    SignedRegValue rhs =  hart->getReg(instr.rs2);

    SignedRegValue res = 0;

    if (lhs < 0 && rhs > 0)
        res = lhs >> rhs | ~(~0U >> rhs);
    else
        res = lhs >> rhs;

    uint32_t res32 = res;
    hart->setReg(instr.rd, sext(res32, 31));
    hart->incrementPC();
}

// ========================== Miscellaneous ============================ //

ALWAYS_INLINE void ExecutorFENCE(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("fence\n");

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorECALL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ecall\n");

    const long syscallNo = hart->getReg(RegisterType::A7);

    switch (syscallNo) {
        case SyscallRV::SYS_WRITE: {
            uint64_t vaddr = hart->getReg(RegisterType::A1);
            uint64_t length = hart->getReg(RegisterType::A2);

            memory::PhysAddr paddr = hart->getPhysAddrR(vaddr);

            std::string outStr;
            outStr.resize(length);

            memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
            pmem.read(paddr, length, outStr.data());
            std::cout << outStr;

            break;
        }
        default: {
            std::cerr << "unknown syscall" << std::endl;
            break;
        }
    }

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorEBREAK(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ebreak\n");

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorFENCEI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorMUL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorMULH(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorMULHSU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorMULHU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorDIV(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("div     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int64_t dividend = hart->getReg(instr.rs1);
    int64_t divisor = hart->getReg(instr.rs2);

    int64_t quot = dividend / divisor;
    hart->setReg(instr.rd, quot);

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorDIVU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorREM(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("rem     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int64_t dividend = hart->getReg(instr.rs1);
    int64_t divisor = hart->getReg(instr.rs2);

    int64_t rem = dividend % divisor;
    hart->setReg(instr.rd, rem);

    hart->incrementPC();
}

ALWAYS_INLINE void ExecutorREMU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorMULW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorDIVW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorDIVUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorREMW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorREMUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOADDW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOXORW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOORW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOANDW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMINW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMAXW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMINUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMAXUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOSWAPW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorLRW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorSCW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOXORD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOORD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOANDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMIND(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMAXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMINUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOMAXUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorAMOSWAPD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorLRD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorSCD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorURET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorSRET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorMRET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorDRET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorSFENCEVMA(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorWFI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorCSRRW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorCSRRS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorCSRRC(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorCSRRWI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorCSRRSI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorCSRRCI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorHFENCEVVMA(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorHFENCEGVMA(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFADDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSUBS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMULS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFDIVS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJNS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJXS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMINS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMAXS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSQRTS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSUBD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMULD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFDIVD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJND(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMIND(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMAXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTSD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSQRTD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFADDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSUBQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMULQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFDIVQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJNQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSGNJXQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMINQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMAXQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTSQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTQS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTQD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSQRTQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLES(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLTS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFEQS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLED(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLTD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFEQD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLEQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLTQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFEQQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTWS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTWUS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTLS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTLUS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMVXW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCLASSS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTWD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTWUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTLD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTLUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMVXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCLASSD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTWQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTWUQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTLQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTLUQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMVXQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCLASSQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTSW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTSWU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTSL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTSLU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMVWX(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTDW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTDWU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTDL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTDLU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMVDX(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTQW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTQWU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTQL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFCVTQLU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMVQX(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFLQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFSQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMADDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMSUBS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFNMSUBS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFNMADDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMSUBD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFNMSUBD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFNMADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMADDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFMSUBQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFNMSUBQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

ALWAYS_INLINE void ExecutorFNMADDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

}  // namespace RISCV
