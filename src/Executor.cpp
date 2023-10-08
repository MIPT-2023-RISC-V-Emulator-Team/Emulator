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

    const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);
}

void ExecutorJALR(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "jalr    x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    const uint64_t returnPC = hart->getPC() + INSTRUCTION_BYTESIZE;
    const uint64_t nextPC = (hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum) & (~1));

    hart->setReg(instr.rd, returnPC);
    hart->setPC(nextPC);
}

// ============================= Branching ============================= //

void ExecutorBEQ(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "beq     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    if (hart->getReg(instr.rs1) == hart->getReg(instr.rs2)) {
        const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBNE(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bne     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    if (hart->getReg(instr.rs1) != hart->getReg(instr.rs2)) {
        const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBLT(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "blt     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    if (static_cast<SignedRegValue>(hart->getReg(instr.rs1)) < hart->getReg(instr.rs2)) {
        const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBGE(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bge     x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    if (static_cast<SignedRegValue>(hart->getReg(instr.rs1)) >= hart->getReg(instr.rs2)) {
        const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBLTU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bltu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    if (hart->getReg(instr.rs1) < hart->getReg(instr.rs2)) {
        const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
        hart->setPC(nextPC);

        return;
    }

    // Increment if no jump
    hart->incrementPC();
}

void ExecutorBGEU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "bgeu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    if (hart->getReg(instr.rs1) >= hart->getReg(instr.rs2)) {
        const uint64_t nextPC = hart->getPC() + sext(instr.imm, instr.immSignBitNum);
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

    hart->mmu_.load8(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, sext(loaded, 7));
    hart->incrementPC();
}

void ExecutorLH(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lh      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint16_t loaded;

    hart->mmu_.load16(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, sext(loaded, 15));
    hart->incrementPC();
}

void ExecutorLW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lw      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint32_t loaded;

    hart->mmu_.load32(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, sext(loaded, 31));
    hart->incrementPC();
}

void ExecutorLD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "ld      x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint64_t loaded;

    hart->mmu_.load64(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

void ExecutorLBU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lbu     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint8_t loaded;

    hart->mmu_.load8(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

void ExecutorLHU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lhu     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint16_t loaded;

    hart->mmu_.load16(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

void ExecutorLWU(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "lwu     x%d, x%d, %ld\n", instr.rd, instr.rs1, sext(instr.imm, instr.immSignBitNum));

    uint32_t loaded;

    hart->mmu_.load32(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), &loaded);
    hart->setReg(instr.rd, loaded);
    hart->incrementPC();
}

// =============================== Store =============================== //

void ExecutorSB(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sb      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint8_t stored = hart->getReg(instr.rs2) & 0xFF;
    hart->mmu_.store8(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), stored);
    hart->incrementPC();
}

void ExecutorSH(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sh      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint16_t stored = hart->getReg(instr.rs2) & 0xFFFF;
    hart->mmu_.store16(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), stored);
    hart->incrementPC();
}

void ExecutorSW(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sw      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint32_t stored = hart->getReg(instr.rs2) & 0xFFFFFFFF;
    hart->mmu_.store32(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), stored);
    hart->incrementPC();
}

void ExecutorSD(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION(
        "sd      x%d, x%d, %ld\n", instr.rs1, instr.rs2, sext(instr.imm, instr.immSignBitNum));

    uint64_t stored = hart->getReg(instr.rs2);
    hart->mmu_.store64(hart->getReg(instr.rs1) + sext(instr.imm, instr.immSignBitNum), stored);
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

    hart->incrementPC();
}

void ExecutorEBREAK(Hart* hart, const DecodedInstruction& instr) {
    DEBUG_INSTRUCTION("ebreak\n");

    hart->incrementPC();
}

}  // namespace RISCV
