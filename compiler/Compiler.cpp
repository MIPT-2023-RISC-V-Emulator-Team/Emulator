#include "compiler/Compiler.h"

#include <asmjit/asmjit.h>

#include "simulator/Hart.h"

namespace RISCV::compiler {

bool Compiler::decrementHotnessCounter(BasicBlock& bb) {
    auto status = bb.getCompilationStatus(std::memory_order_acquire);
    switch (status) {
        case CompilationStatus::NOT_COMPILED:
            break;
        case CompilationStatus::COMPILING:
            return true;
        case CompilationStatus::COMPILED:
            return false;
        default:
            UNREACHABLE();
    }

    if (bb.decrementHotnessCounter()) {
        return true;
    }

    bb.setCompilationStatus(CompilationStatus::COMPILING, std::memory_order_relaxed);
    CompilerTask compiler_task{bb.getBody(), bb.getEntrypoint()};
    worker_.addTask(std::move(compiler_task));
    return true;
}

void Compiler::compileBasicBlock(CompilerTask&& task) {
    CompiledEntry entry = nullptr;
    for (auto&& instr : task.instrs) {
        generateInstr(&entry, instr);
    }
    hart_->setBBEntry(task.entrypoint, entry);
}

void Compiler::generateInstr([[maybe_unused]] CompiledEntry* entry,
                             [[maybe_unused]] const DecodedInstruction& instr) {
    // TODO(all): implement codegen
    // asmjit::JitRuntime rt;
    // asmjit::CodeHolder code;
    // switch (instr.type)
}

}  // namespace RISCV::compiler
