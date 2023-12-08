#include "compiler/Codegen.h"

#include "simulator/Hart.h"

namespace RISCV::compiler {

using namespace asmjit;

void CodeGenerator::initialize() {
    static auto entry_signature = FuncSignatureT<void, Hart *, const DecodedInstruction *>();
    auto *entry_node = compiler_.addFunc(entry_signature);

    hart_p_ = compiler_.newGpq();
    instr_p_ = compiler_.newGpq();
    entry_node->setArg(0, hart_p_);
    entry_node->setArg(1, instr_p_);

    pc_p_ = compiler_.newGpq();
    compiler_.mov(pc_p_, hart_p_);
    compiler_.add(pc_p_, Hart::getOffsetToPc());

    regs_p_ = compiler_.newGpq();
    compiler_.mov(regs_p_, hart_p_);
    compiler_.add(regs_p_, Hart::getOffsetToRegs());
}

void CodeGenerator::finalize() {
    compiler_.endFunc();
    compiler_.finalize();
}

x86::Gp CodeGenerator::generateGetReg(size_t index) {
    auto reg = compiler_.newGpq();
    compiler_.mov(reg, x86::qword_ptr(regs_p_, sizeof(RegValue) * index));
    return reg;
}

void CodeGenerator::generateSetReg(size_t index, uint64_t imm) {
    compiler_.mov(x86::qword_ptr(regs_p_, sizeof(RegValue) * index), imm);
}

void CodeGenerator::generateSetReg(size_t index, x86::Gp reg) {
    compiler_.mov(x86::qword_ptr(regs_p_, sizeof(RegValue) * index), reg);
}

x86::Gp CodeGenerator::generateGetPC() {
    // TODO(panferovi): pin PC and hart->regs_ to registers
    auto pc = compiler_.newGpq();
    compiler_.mov(pc, x86::qword_ptr(pc_p_));
    return pc;
}

void CodeGenerator::generateSetPC(uint64_t imm) {
    compiler_.mov(x86::qword_ptr(pc_p_), imm);
}

void CodeGenerator::generateSetPC(x86::Gp pc) {
    compiler_.mov(x86::qword_ptr(pc_p_), pc);
}

void CodeGenerator::generateIncrementPC() {
    compiler_.inc(x86::qword_ptr(pc_p_));
}

void CodeGenerator::generateInvoke(Executor executor, size_t instr_offest) {
    auto instr = compiler_.newGpq();
    compiler_.mov(instr, instr_p_);
    compiler_.add(instr, sizeof(DecodedInstruction) * instr_offest);

    static auto executor_signature = FuncSignatureT<void, Hart *, const DecodedInstruction &>();
    asmjit::InvokeNode *invokeNode = nullptr;
    compiler_.invoke(&invokeNode, executor, executor_signature);
    invokeNode->setArg(0, hart_p_);
    invokeNode->setArg(1, instr);
}

void ExecutorPrint(Hart *hart, const char *str, uint64_t reg) {
    std::cout << str << "\n";
    std::cout << "print reg: " << reg << "\n";
}

void CodeGenerator::generatePrint(const char *str, x86::Gp reg) {
    static auto print = FuncSignatureT<void, Hart *, const char *, uint64_t>();
    asmjit::InvokeNode *invokeNode = nullptr;
    compiler_.invoke(&invokeNode, ExecutorPrint, print);
    invokeNode->setArg(0, hart_p_);
    invokeNode->setArg(1, str);
    invokeNode->setArg(2, reg);
}

void CodeGenerator::generateLUI(const DecodedInstruction &instr) {
    generateSetReg(instr.rd, instr.imm);
    generateIncrementPC();
}

void CodeGenerator::generateAUIPC(const DecodedInstruction &instr) {
    auto pc = generateGetPC();
    compiler_.add(pc, instr.imm);
    generateSetReg(instr.rd, pc);
    generateIncrementPC();
}

void CodeGenerator::generateJAL(const DecodedInstruction &instr) {
    auto pc = generateGetPC();

    auto tmp = compiler_.newGpq();
    compiler_.mov(tmp, pc);
    compiler_.add(tmp, INSTRUCTION_BYTESIZE);
    generateSetReg(instr.rd, tmp);

    compiler_.add(pc, instr.imm);
    generateSetPC(pc);
}

void CodeGenerator::generateJALR(const DecodedInstruction &instr) {
    auto returnPC = generateGetPC();
    compiler_.add(returnPC, INSTRUCTION_BYTESIZE);

    auto nextPC = generateGetReg(instr.rs1);
    compiler_.add(nextPC, instr.imm);
    compiler_.and_(nextPC, ~1U);

    generateSetReg(instr.rd, returnPC);
    generateSetPC(nextPC);
}

}  // namespace RISCV::compiler
