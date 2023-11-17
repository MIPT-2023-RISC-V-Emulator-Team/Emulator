#ifndef INCLUDE_COMPILER_H_
#define INCLUDE_COMPILER_H_

#include "BasicBlock.h"
#include "CompilerWorker.h"

namespace RISCV {

class Hart;

class Compiler {
public:
    using CompiledEntry = BasicBlock::CompiledEntry;

    Compiler(Hart* hart) : hart_(hart), worker_(this) {}

    void InitializeWorker() {
        worker_.Initialize();
    }

    void FinalizeWorker() {
        worker_.Finalize();
    }

    bool decrementHotnessCounter(BasicBlock& bb);
    void compileBasicBlock(CompilerTask&& task);
    void generateInstr(CompiledEntry* entry, const DecodedInstruction& instr);

private:
    Hart* hart_;
    CompilerWorker worker_;
};

}  // namespace RISCV

#endif  // INCLUDE_COMPILER_H_
