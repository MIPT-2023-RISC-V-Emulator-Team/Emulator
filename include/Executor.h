#ifndef INCLUDE_EXECUTOR_H
#define INCLUDE_EXECUTOR_H

namespace RISCV {

class Hart;
struct DecodedInstruction;

// ================================ PC ================================= //

void ExecutorLUI(Hart* hart, const DecodedInstruction& instr);
void ExecutorAUIPC(Hart* hart, const DecodedInstruction& instr);

// =============================== Jumps =============================== //

void ExecutorJAL(Hart* hart, const DecodedInstruction& instr);
void ExecutorJALR(Hart* hart, const DecodedInstruction& instr);

// ============================= Branching ============================= //

void ExecutorBEQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorBNE(Hart* hart, const DecodedInstruction& instr);
void ExecutorBLT(Hart* hart, const DecodedInstruction& instr);
void ExecutorBGE(Hart* hart, const DecodedInstruction& instr);
void ExecutorBLTU(Hart* hart, const DecodedInstruction& instr);
void ExecutorBGEU(Hart* hart, const DecodedInstruction& instr);

// =============================== Load ================================ //

void ExecutorLB(Hart* hart, const DecodedInstruction& instr);
void ExecutorLH(Hart* hart, const DecodedInstruction& instr);
void ExecutorLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorLD(Hart* hart, const DecodedInstruction& instr);
void ExecutorLBU(Hart* hart, const DecodedInstruction& instr);
void ExecutorLHU(Hart* hart, const DecodedInstruction& instr);
void ExecutorLWU(Hart* hart, const DecodedInstruction& instr);

// =============================== Store =============================== //

void ExecutorSB(Hart* hart, const DecodedInstruction& instr);
void ExecutorSH(Hart* hart, const DecodedInstruction& instr);
void ExecutorSW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSD(Hart* hart, const DecodedInstruction& instr);

// ======================= Arithmetic immediate ======================== //

void ExecutorADDI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLLI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLTI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLTIU(Hart* hart, const DecodedInstruction& instr);
void ExecutorXORI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRLI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRAI(Hart* hart, const DecodedInstruction& instr);
void ExecutorORI(Hart* hart, const DecodedInstruction& instr);
void ExecutorANDI(Hart* hart, const DecodedInstruction& instr);
void ExecutorADDIW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLLIW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRLIW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRAIW(Hart* hart, const DecodedInstruction& instr);

// ============================ Arithmetic ============================= //

void ExecutorADD(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLL(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLT(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLTU(Hart* hart, const DecodedInstruction& instr);
void ExecutorXOR(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRL(Hart* hart, const DecodedInstruction& instr);
void ExecutorOR(Hart* hart, const DecodedInstruction& instr);
void ExecutorAND(Hart* hart, const DecodedInstruction& instr);
void ExecutorSUB(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRA(Hart* hart, const DecodedInstruction& instr);
void ExecutorADDW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSUBW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRAW(Hart* hart, const DecodedInstruction& instr);

// ========================== Miscellaneous ============================ //

void ExecutorFENCE(Hart* hart, const DecodedInstruction& instr);
void ExecutorECALL(Hart* hart, const DecodedInstruction& instr);
void ExecutorEBREAK(Hart* hart, const DecodedInstruction& instr);

}  // namespace RISCV

#endif  // INCLUDE_EXECUTOR_H
