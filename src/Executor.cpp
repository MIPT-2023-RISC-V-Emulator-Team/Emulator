#include "Executor.h"

#include "Hart.h"
#include "macros.h"

namespace RISCV {

// Sign Extend. signBitNum = 0, 1, 2, ..., 31 from right to left
static inline uint64_t sext(const uint32_t val, const uint8_t signBitNum) {
    const uint64_t tmp = val;
    return ~(((tmp & (1 << signBitNum)) - 1)) | tmp;
}

// Zero Extend
static inline uint64_t zext(const uint32_t val) {
    const uint64_t tmp = val;
    return tmp;
}

// ================================ PC ================================= //

void ExecutorLUI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("lui     x%d, %ld\n", instr.rd, sext(instr.imm, instr.immSignBitNum));

    hart->setReg(instr.rd, sext(instr.imm, instr.immSignBitNum));
    hart->incrementPC();
}

void ExecutorAUIPC(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("auipc   x%d, %ld\n", instr.rd, sext(instr.imm, instr.immSignBitNum));

    hart->setReg(instr.rd, hart->getPC() + sext(instr.imm, instr.immSignBitNum));
    hart->incrementPC();
}

// =============================== Jumps =============================== //

void ExecutorJAL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("jal     x%d, %ld\n", instr.rd, sext(instr.imm, instr.immSignBitNum));

    hart->setReg(instr.rd, hart->getPC() + INSTRUCTION_BYTESIZE);

    const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);
}

void ExecutorJALR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "jalr    x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    const memory::VirtAddr returnPC = hart->getPC() + INSTRUCTION_BYTESIZE;
    const memory::VirtAddr nextPC = (hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum)) & ~1;

    hart->setReg(instr.rd, returnPC);
    hart->setPC(nextPC);
}

// ============================= Branching ============================= //

void ExecutorBEQ(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "beq     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs == rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBNE(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bne     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs != rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBLT(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "blt     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs < rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBGE(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bge     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    const SignedRegValue lhs = hart->getReg(instr.rs1);
    const SignedRegValue rhs = hart->getReg(instr.rs2);

    if (lhs >= rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBLTU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bltu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    const RegValue lhs = hart->getReg(instr.rs1);
    const RegValue rhs = hart->getReg(instr.rs2);

    if (lhs < rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBGEU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bgeu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    const RegValue lhs = hart->getReg(instr.rs1);
    const RegValue rhs = hart->getReg(instr.rs2);

    if (lhs >= rhs) {
        const memory::VirtAddr nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

// =============================== Load ================================ //

void ExecutorLB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lb      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint8_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 1, &loaded);

    hart->setReg(instr.rd, sext(loaded, 7));
    hart->incrementPC();
}

void ExecutorLH(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lh      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint16_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 2, &loaded);

    hart->setReg(instr.rd, sext(loaded, 15));
    hart->incrementPC();
}

void ExecutorLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lw      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint32_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 4, &loaded);

    hart->setReg(instr.rd, sext(loaded, 31));
    hart->incrementPC();
}

void ExecutorLD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "ld      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint64_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 8, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

void ExecutorLBU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lbu     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint8_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 1, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

void ExecutorLHU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lhu     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint16_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 2, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

void ExecutorLWU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lwu     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint32_t loaded;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findR(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrR(vaddr);
        tlb.insertR(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, 4, &loaded);

    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

// =============================== Store =============================== //

void ExecutorSB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sb      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint8_t stored = hart->getReg(instr.rs2) & 0xFF;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findW(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrW(vaddr);
        tlb.insertW(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 1, &stored);

    hart->incrementPC();
}

void ExecutorSH(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sh      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint16_t stored = hart->getReg(instr.rs2) & 0xFFFF;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findW(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrW(vaddr);
        tlb.insertW(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 2, &stored);

    hart->incrementPC();
}

void ExecutorSW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sw      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint32_t stored = hart->getReg(instr.rs2) & 0xFFFFFFFF;

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findW(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrW(vaddr);
        tlb.insertW(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 4, &stored);

    hart->incrementPC();
}

void ExecutorSD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sd      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint64_t stored = hart->getReg(instr.rs2);

    memory::VirtAddr vaddr = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    memory::PhysAddr paddr;


    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(vaddr);
    auto& tlb = hart->getTLB();
    auto tlbEntry = tlb.findW(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
        paddr += memory::getPageOffset(vaddr);
    }
    else {
        // TLB miss, translate address in usual way
        auto& mmu = hart->getTranslator();
        paddr = mmu.getPhysAddrW(vaddr);
        tlb.insertW(vpn, memory::getPageNumber(paddr));
    }


    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.write(paddr, 8, &stored);

    hart->incrementPC();
}

// ======================= Arithmetic immediate ======================== //

void ExecutorADDI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "addi    x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint64_t sum = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    hart->setReg(instr.rd, sum);
    hart->incrementPC();
}

void ExecutorSLLI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    uint64_t result = hart->getReg(instr.rs1) << instr.shamt;
    hart->setReg(instr.rd, result);

    hart->incrementPC();
}

void ExecutorSLTI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slti    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSLTIU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sltiu   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorXORI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "xori    x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    hart->incrementPC();
}

void ExecutorSRLI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    hart->incrementPC();
}

void ExecutorSRAI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srai    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    hart->incrementPC();
}

void ExecutorORI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "ori     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    hart->incrementPC();
}

void ExecutorANDI(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "andi    x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    hart->incrementPC();
}

void ExecutorADDIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "addiw   x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint64_t sum = hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum);
    uint32_t sum32 = sum;

    hart->setReg(instr.rd, sext(sum32, 31));

    hart->incrementPC();
}

void ExecutorSLLIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    hart->incrementPC();
}

void ExecutorSRLIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    hart->incrementPC();
}

void ExecutorSRAIW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sraiw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);

    hart->incrementPC();
}

// ============================ Arithmetic ============================= //

void ExecutorADD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("add     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->setReg(instr.rd, hart->getReg(instr.rs1) + hart->getReg(instr.rs2));

    hart->incrementPC();
}

void ExecutorSLL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sll     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSLT(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("slt     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSLTU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sltu    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorXOR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("xor     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSRL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srl     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorOR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("or      x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorAND(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("and     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSUB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sub     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSRA(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sra     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorADDW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("addw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    uint64_t sum = hart->getReg(instr.rs1) + hart->getReg(instr.rs2);
    uint32_t sum32 = sum;

    hart->setReg(instr.rd, sext(sum32, 31));

    hart->incrementPC();
}

void ExecutorSUBW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("subw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSLLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sllw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSRLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("srlw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

void ExecutorSRAW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("sraw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    hart->incrementPC();
}

// ========================== Miscellaneous ============================ //

void ExecutorFENCE(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("fence\n");

    hart->incrementPC();
}

void ExecutorECALL(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ecall\n");

    const long syscallNo = hart->getReg(RegisterType::A7);

    switch (syscallNo) {
        case 64: {
            uint64_t vaddr = hart->getReg(RegisterType::A1);
            uint64_t length = hart->getReg(RegisterType::A2);

            memory::PhysAddr paddr;

            // Try TLB
            const uint64_t vpn = getPartialBits<12, 63>(vaddr);
            auto& tlb = hart->getTLB();
            auto tlbEntry = tlb.findR(vpn);
            if (tlbEntry != std::nullopt) {
                // TLB hit
                paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
                paddr += memory::getPageOffset(vaddr);
            }
            else {
                // TLB miss, translate address in usual way
                auto& mmu = hart->getTranslator();
                paddr = mmu.getPhysAddrR(vaddr);
                tlb.insertR(vpn, memory::getPageNumber(paddr));
            }

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

void ExecutorEBREAK(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ebreak\n");

    hart->incrementPC();
}

void ExecutorFENCEI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorMUL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorMULH(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorMULHSU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorMULHU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorDIV(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("div     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int64_t dividend = hart->getReg(instr.rs1);
    int64_t divisor = hart->getReg(instr.rs2);

    int64_t quot = dividend / divisor;
    hart->setReg(instr.rd, quot);

    hart->incrementPC();
}

void ExecutorDIVU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorREM(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("rem     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);

    int64_t dividend = hart->getReg(instr.rs1);
    int64_t divisor = hart->getReg(instr.rs2);

    int64_t rem = dividend % divisor;
    hart->setReg(instr.rd, rem);

    hart->incrementPC();
}

void ExecutorREMU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorMULW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorDIVW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorDIVUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorREMW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorREMUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOADDW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOXORW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOORW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOANDW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMINW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMAXW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMINUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMAXUW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOSWAPW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorLRW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorSCW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOXORD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOORD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOANDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMIND(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMAXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMINUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOMAXUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorAMOSWAPD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorLRD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorSCD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorURET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorSRET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorMRET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorDRET(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorSFENCEVMA(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorWFI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorCSRRW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorCSRRS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorCSRRC(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorCSRRWI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorCSRRSI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorCSRRCI(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorHFENCEVVMA(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorHFENCEGVMA(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFADDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSUBS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMULS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFDIVS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJNS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJXS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMINS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMAXS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSQRTS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSUBD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMULD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFDIVD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJND(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMIND(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMAXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTSD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSQRTD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFADDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSUBQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMULQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFDIVQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJNQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSGNJXQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMINQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMAXQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTSQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTQS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTQD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSQRTQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLES(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLTS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFEQS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLED(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLTD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFEQD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLEQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLTQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFEQQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTWS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTWUS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTLS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTLUS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMVXW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCLASSS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTWD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTWUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTLD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTLUD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMVXD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCLASSD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTWQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTWUQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTLQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTLUQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMVXQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCLASSQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTSW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTSWU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTSL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTSLU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMVWX(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTDW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTDWU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTDL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTDLU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMVDX(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTQW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTQWU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTQL(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFCVTQLU(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMVQX(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFLQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSW(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFSQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMADDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMSUBS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFNMSUBS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFNMADDS(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMSUBD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFNMSUBD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFNMADDD(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMADDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFMSUBQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFNMSUBQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

void ExecutorFNMADDQ(Hart* hart, const DecodedInstruction& instr) {
    UNREACHABLE();
}

}  // namespace RISCV
