#ifndef INCLUDE_COMPILER_H_
#define INCLUDE_COMPILER_H_

#include "compiler/CompilerWorker.h"
#include "simulator/BasicBlock.h"

namespace RISCV {
class Hart;
}  // namespace RISCV

namespace RISCV::compiler {

class Compiler {
public:
    using CompiledEntry = BasicBlock::CompiledEntry;

    Compiler(Hart *hart) : hart_(hart), worker_(this) {}

    void InitializeWorker() {
        worker_.Initialize();
    }

    void FinalizeWorker() {
        worker_.Finalize();
    }

    bool decrementHotnessCounter(BasicBlock &bb);
    void compileBasicBlock(CompilerTask &&task);
    void generateInstr(CompiledEntry *entry, const DecodedInstruction &instr);

private:
    Hart *hart_;
    CompilerWorker worker_;
};

}  // namespace RISCV::compiler

#endif  // INCLUDE_COMPILER_H_
