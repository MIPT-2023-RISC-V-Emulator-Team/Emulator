#ifndef CODEGEN_H
#define CODEGEN_H

#include <asmjit/asmjit.h>

#include "simulator/Executor.h"

namespace RISCV::compiler {

class Compiler;

class CodeGenerator {
public:
    CodeGenerator(asmjit::CodeHolder *code) : compiler_(code) {}

    void initialize();
    void finalize();

    void generateInvoke(Executor executor, size_t instr_offest);

    void generateLUI(const DecodedInstruction &instr);
    void generateAUIPC(const DecodedInstruction &instr);
    void generateJAL(const DecodedInstruction &instr);
    void generateJALR(const DecodedInstruction &instr);

private:
    asmjit::x86::Gp generateGetReg(size_t index);
    void generateSetReg(size_t index, uint64_t imm);
    void generateSetReg(size_t index, asmjit::x86::Gp reg);

    asmjit::x86::Gp generateGetPC();
    void generateSetPC(uint64_t imm);
    void generateSetPC(asmjit::x86::Gp reg);
    void generateIncrementPC();

    void generatePrint(const char *str, asmjit::x86::Gp reg);

    asmjit::x86::Compiler compiler_;
    asmjit::x86::Gp hart_p_;
    asmjit::x86::Gp pc_p_;
    asmjit::x86::Gp regs_p_;
    asmjit::x86::Gp instr_p_;
};

}  // namespace RISCV::compiler

#endif  // CODEGEN_H
