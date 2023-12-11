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

    pc_ = compiler_.newGpq();
    compiler_.mov(pc_, x86::qword_ptr(hart_p_, Hart::getOffsetToPc()));
}

void CodeGenerator::finalize() {
    generatePutRegsAndPCInMem();
    compiler_.endFunc();
    compiler_.finalize();
}

x86::Gp CodeGenerator::generateGetReg(size_t index) {
    auto reg_iter = register_map_.find(index);
    if (reg_iter != register_map_.cend()) {
        return reg_iter->second;
    }

    auto reg = compiler_.newGpq();
    compiler_.mov(reg, x86::qword_ptr(hart_p_, Hart::getOffsetToRegs() + sizeof(RegValue) * index));
    register_map_.insert(std::pair(index, reg));
    return reg;
}

void CodeGenerator::generateSetReg(size_t index, uint64_t imm) {
    auto reg_iter = register_map_.find(index);
    if (reg_iter != register_map_.cend()) {
        compiler_.mov(reg_iter->second, imm);
    } else {
        auto reg = compiler_.newGpq();
        compiler_.mov(reg, imm);
        register_map_.insert(std::pair(index, reg));
    }
}

void CodeGenerator::generateSetReg(size_t index, x86::Gp reg) {
    register_map_[index] = reg;
}

x86::Gp CodeGenerator::generateGetPC() {
    return pc_;
}

void CodeGenerator::generateSetPC(uint64_t imm) {
    compiler_.mov(pc_, imm);
}

void CodeGenerator::generateSetPC(x86::Gp pc) {
    compiler_.mov(pc_, pc);
}

void CodeGenerator::generateIncrementPC() {
    compiler_.add(pc_, INSTRUCTION_BYTESIZE);
}

void CodeGenerator::generatePutRegsAndPCInMem() {
    for (auto &&reg : register_map_) {
        if (reg.first == 0) {
            continue;
        }
        auto reg_offset = Hart::getOffsetToRegs() + sizeof(RegValue) * reg.first;
        compiler_.mov(x86::qword_ptr(hart_p_, reg_offset), reg.second);
    }
    register_map_.clear();

    // compiler_.mov(x86::qword_ptr(hart_p_, Hart::getOffsetToPc()), pc_);
}

void CodeGenerator::generateInvoke(Executor executor, size_t instr_offest) {
    generatePutRegsAndPCInMem();

    auto instr = compiler_.newGpq();
    compiler_.mov(instr, instr_p_);
    compiler_.add(instr, sizeof(DecodedInstruction) * instr_offest);

    static auto executor_signature = FuncSignatureT<void, Hart *, const DecodedInstruction &>();
    asmjit::InvokeNode *invokeNode = nullptr;
    compiler_.invoke(&invokeNode, executor, executor_signature);
    invokeNode->setArg(0, hart_p_);
    invokeNode->setArg(1, instr);

    // compiler_.mov(pc_, x86::qword_ptr(hart_p_, Hart::getOffsetToPc()));
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
