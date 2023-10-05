#include "Executor.h"

#include "Hart.h"

//#define DEBUG_INSTRUCTIONS

namespace RISCV {

// signBitNum = 0, 1, 2, ..., 31 from right to left
static inline uint64_t signExtend(const uint64_t val, const uint8_t signBitNum) {
  return ~(((val & (1 << signBitNum)) - 1)) | val;
}

// ================================ PC ================================= //

void ExecutorLUI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("lui     x%d, %ld\n", instr.rd, signExtend(instr.imm, instr.immSignBitNum));
#endif

  hart->setReg(instr.rd, signExtend(instr.imm, instr.immSignBitNum));
  hart->incrementPC();
}

void ExecutorAUIPC::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("auipc   x%d, %ld\n", instr.rd, signExtend(instr.imm, instr.immSignBitNum));
#endif

  hart->setReg(instr.rd, hart->getPC() + signExtend(instr.imm, instr.immSignBitNum));
  hart->incrementPC();
}

// =============================== Jumps =============================== //

void ExecutorJAL::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("jal     x%d, %ld\n", instr.rd, signExtend(instr.imm, instr.immSignBitNum));
#endif

  hart->setReg(instr.rd, hart->getPC() + INSTRUCTION_BYTESIZE);

  const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
  hart->setPC(nextPC);
}

void ExecutorJALR::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "jalr    x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  const uint64_t returnPC = hart->getPC() + INSTRUCTION_BYTESIZE;
  const uint64_t nextPC =
      (hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum) & (~1));

  hart->setReg(instr.rd, returnPC);
  hart->setPC(nextPC);
}

// ============================= Branching ============================= //

void ExecutorBEQ::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "beq     x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  if (hart->getReg(instr.rs1) == hart->getReg(instr.rs2)) {
    const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);

    return;
  }

  // Increment if no jump
  hart->incrementPC();
}

void ExecutorBNE::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "bne     x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  if (hart->getReg(instr.rs1) != hart->getReg(instr.rs2)) {
    const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);

    return;
  }

  // Increment if no jump
  hart->incrementPC();
}

void ExecutorBLT::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "blt     x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  if (static_cast<SignedRegValue>(hart->getReg(instr.rs1)) < hart->getReg(instr.rs2)) {
    const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);

    return;
  }

  // Increment if no jump
  hart->incrementPC();
}

void ExecutorBGE::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "bge     x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  if (static_cast<SignedRegValue>(hart->getReg(instr.rs1)) >= hart->getReg(instr.rs2)) {
    const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);

    return;
  }

  // Increment if no jump
  hart->incrementPC();
}

void ExecutorBLTU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "bltu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  if (hart->getReg(instr.rs1) < hart->getReg(instr.rs2)) {
    const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);

    return;
  }

  // Increment if no jump
  hart->incrementPC();
}

void ExecutorBGEU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "bgeu    x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  if (hart->getReg(instr.rs1) >= hart->getReg(instr.rs2)) {
    const uint64_t nextPC = hart->getPC() + signExtend(instr.imm, instr.immSignBitNum);
    hart->setPC(nextPC);

    return;
  }

  // Increment if no jump
  hart->incrementPC();
}

// =============================== Load ================================ //

void ExecutorLB::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "lb      x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint8_t loaded;

  hart->mmu_.load8(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, signExtend(loaded, 7));
  hart->incrementPC();
}

void ExecutorLH::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "lh      x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint16_t loaded;

  hart->mmu_.load16(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, signExtend(loaded, 15));
  hart->incrementPC();
}

void ExecutorLW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "lw      x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint32_t loaded;

  hart->mmu_.load32(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, signExtend(loaded, 31));
  hart->incrementPC();
}

void ExecutorLD::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "ld      x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint64_t loaded;

  hart->mmu_.load64(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, loaded);
  hart->incrementPC();
}

void ExecutorLBU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "lbu     x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint8_t loaded;

  hart->mmu_.load8(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, loaded);
  hart->incrementPC();
}

void ExecutorLHU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "lhu     x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint16_t loaded;

  hart->mmu_.load16(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, loaded);
  hart->incrementPC();
}

void ExecutorLWU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "lwu     x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint32_t loaded;

  hart->mmu_.load32(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), &loaded);
  hart->setReg(instr.rd, loaded);
  hart->incrementPC();
}

// =============================== Store =============================== //

void ExecutorSB::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "sb      x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint8_t stored = hart->getReg(instr.rs2) & 0xFF;
  hart->mmu_.store8(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), stored);
  hart->incrementPC();
}

void ExecutorSH::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "sh      x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint16_t stored = hart->getReg(instr.rs2) & 0xFFFF;
  hart->mmu_.store16(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), stored);
  hart->incrementPC();
}

void ExecutorSW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "sw      x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint32_t stored = hart->getReg(instr.rs2) & 0xFFFFFFFF;
  hart->mmu_.store32(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), stored);
  hart->incrementPC();
}

void ExecutorSD::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "sd      x%d, x%d, %ld\n", instr.rs1, instr.rs2, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint64_t stored = hart->getReg(instr.rs2);
  hart->mmu_.store64(hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum), stored);
  hart->incrementPC();
}

// ======================= Arithmetic immediate ======================== //

void ExecutorADDI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "addi    x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint64_t sum = hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum);
  hart->setReg(instr.rd, sum);
  hart->incrementPC();
}

void ExecutorSLLI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("slli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);
#endif

  hart->incrementPC();
}

void ExecutorSLTI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("slti    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSLTIU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sltiu   x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorXORI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "xori    x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  hart->incrementPC();
}

void ExecutorSRLI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("srli    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);
#endif

  hart->incrementPC();
}

void ExecutorSRAI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("srai    x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);
#endif

  hart->incrementPC();
}

void ExecutorORI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "ori     x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  hart->incrementPC();
}

void ExecutorANDI::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "andi    x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  hart->incrementPC();
}

void ExecutorADDIW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf(
      "addiw   x%d, x%d, %ld\n", instr.rd, instr.rs1, signExtend(instr.imm, instr.immSignBitNum));
#endif

  uint64_t sum = hart->getReg(instr.rs1) + signExtend(instr.imm, instr.immSignBitNum);
  uint32_t sum32 = sum;

  hart->setReg(instr.rd, signExtend(sum32, 31));

  hart->incrementPC();
}

void ExecutorSLLIW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("slliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);
#endif

  hart->incrementPC();
}

void ExecutorSRLIW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("srliw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);
#endif

  hart->incrementPC();
}

void ExecutorSRAIW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sraiw   x%d, x%d, %d\n", instr.rd, instr.rs1, instr.shamt);
#endif

  hart->incrementPC();
}

// ============================ Arithmetic ============================= //

void ExecutorADD::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("add     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSLL::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sll     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSLT::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("slt     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSLTU::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sltu    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorXOR::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("xor     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSRL::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("srl     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorOR::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("or      x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorAND::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("and     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSUB::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sub     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSRA::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sra     x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorADDW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("addw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  uint64_t sum = hart->getReg(instr.rs1) + hart->getReg(instr.rs2);
  uint32_t sum32 = sum;

  hart->setReg(instr.rd, signExtend(sum32, 31));

  hart->incrementPC();
}

void ExecutorSUBW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("subw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSLLW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sllw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSRLW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("srlw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

void ExecutorSRAW::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("sraw    x%d, x%d, x%d\n", instr.rd, instr.rs1, instr.rs2);
#endif

  hart->incrementPC();
}

// ========================== Miscellaneous ============================ //

void ExecutorFENCE::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("fence\n");
#endif

  hart->incrementPC();
}

void ExecutorECALL::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("ecall\n");
#endif

  hart->incrementPC();
}

void ExecutorEBREAK::operator()(Hart* hart, const DecodedInstruction& instr) {
#ifdef DEBUG_INSTRUCTIONS
  printf("ebreak\n");
#endif

  hart->incrementPC();
}

// ===================================================================== //
// ========================= Singleton init ============================ //
// ===================================================================== //

// ================================ PC ================================= //

MAKE_EXECUTOR_INIT(LUI)
MAKE_EXECUTOR_INIT(AUIPC)

// =============================== Jumps =============================== //

MAKE_EXECUTOR_INIT(JAL)
MAKE_EXECUTOR_INIT(JALR)

// ============================= Branching ============================= //

MAKE_EXECUTOR_INIT(BEQ)
MAKE_EXECUTOR_INIT(BNE)
MAKE_EXECUTOR_INIT(BLT)
MAKE_EXECUTOR_INIT(BGE)
MAKE_EXECUTOR_INIT(BLTU)
MAKE_EXECUTOR_INIT(BGEU)

// =============================== Load ================================ //

MAKE_EXECUTOR_INIT(LB)
MAKE_EXECUTOR_INIT(LH)
MAKE_EXECUTOR_INIT(LW)
MAKE_EXECUTOR_INIT(LD)
MAKE_EXECUTOR_INIT(LBU)
MAKE_EXECUTOR_INIT(LHU)
MAKE_EXECUTOR_INIT(LWU)

// =============================== Store =============================== //

MAKE_EXECUTOR_INIT(SB)
MAKE_EXECUTOR_INIT(SH)
MAKE_EXECUTOR_INIT(SW)
MAKE_EXECUTOR_INIT(SD)

// ======================= Arithmetic immediate ======================== //

MAKE_EXECUTOR_INIT(ADDI)
MAKE_EXECUTOR_INIT(SLLI)
MAKE_EXECUTOR_INIT(SLTI)
MAKE_EXECUTOR_INIT(SLTIU)
MAKE_EXECUTOR_INIT(XORI)
MAKE_EXECUTOR_INIT(SRLI)
MAKE_EXECUTOR_INIT(SRAI)
MAKE_EXECUTOR_INIT(ORI)
MAKE_EXECUTOR_INIT(ANDI)
MAKE_EXECUTOR_INIT(ADDIW)
MAKE_EXECUTOR_INIT(SLLIW)
MAKE_EXECUTOR_INIT(SRLIW)
MAKE_EXECUTOR_INIT(SRAIW)

// ============================ Arithmetic ============================= //

MAKE_EXECUTOR_INIT(ADD)
MAKE_EXECUTOR_INIT(SLL)
MAKE_EXECUTOR_INIT(SLT)
MAKE_EXECUTOR_INIT(SLTU)
MAKE_EXECUTOR_INIT(XOR)
MAKE_EXECUTOR_INIT(SRL)
MAKE_EXECUTOR_INIT(OR)
MAKE_EXECUTOR_INIT(AND)
MAKE_EXECUTOR_INIT(SUB)
MAKE_EXECUTOR_INIT(SRA)
MAKE_EXECUTOR_INIT(ADDW)
MAKE_EXECUTOR_INIT(SUBW)
MAKE_EXECUTOR_INIT(SLLW)
MAKE_EXECUTOR_INIT(SRLW)
MAKE_EXECUTOR_INIT(SRAW)

// ========================== Miscellaneous ============================ //

MAKE_EXECUTOR_INIT(FENCE)
MAKE_EXECUTOR_INIT(ECALL)
MAKE_EXECUTOR_INIT(EBREAK)

}  // namespace RISCV
