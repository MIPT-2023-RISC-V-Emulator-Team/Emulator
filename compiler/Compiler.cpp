#include "compiler/Compiler.h"

#include <asmjit/asmjit.h>

#include "compiler/Codegen.h"
#include "generated/InstructionTypes.h"
#include "simulator/Executor-inl.h"
#include "simulator/Hart.h"

namespace RISCV::compiler {

using namespace asmjit;

bool Compiler::decrementHotnessCounter(BasicBlock &bb) {
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

void Compiler::compileBasicBlock(CompilerTask &&task) {
    CodeHolder code;
    code.init(runtime_.environment(), runtime_.cpuFeatures());
    CodeGenerator codegen(&code);
    codegen.initialize();

    for (size_t i = 0; i < task.instrs.size(); ++i) {
        if (task.instrs[i].type != InstructionType::BASIC_BLOCK_END) {
            generateInstr(codegen, task.instrs[i], i);
        }
    }

    codegen.finalize();
    CompiledEntry entry = nullptr;
    runtime_.add(&entry, &code);
    hart_->setBBEntry(task.entrypoint, entry);
}

void Compiler::generateInstr(CodeGenerator &codegen, const DecodedInstruction &instr, size_t instr_offset) {
    switch (instr.type) {
        case InstructionType::LUI:
            codegen.generateInvoke(ExecutorLUI, instr_offset);
            return;
        case InstructionType::AUIPC:
            codegen.generateInvoke(ExecutorAUIPC, instr_offset);
            return;
        case InstructionType::JAL:
            codegen.generateInvoke(ExecutorJAL, instr_offset);
            return;
        case InstructionType::JALR:
            codegen.generateInvoke(ExecutorJALR, instr_offset);
            return;
        case InstructionType::BEQ:
            codegen.generateInvoke(ExecutorBEQ, instr_offset);
            return;
        case InstructionType::BNE:
            codegen.generateInvoke(ExecutorBNE, instr_offset);
            return;
        case InstructionType::BLT:
            codegen.generateInvoke(ExecutorBLT, instr_offset);
            return;
        case InstructionType::BGE:
            codegen.generateInvoke(ExecutorBGE, instr_offset);
            return;
        case InstructionType::BLTU:
            codegen.generateInvoke(ExecutorBLTU, instr_offset);
            return;
        case InstructionType::BGEU:
            codegen.generateInvoke(ExecutorBGEU, instr_offset);
            return;
        case InstructionType::LB:
            codegen.generateInvoke(ExecutorLB, instr_offset);
            return;
        case InstructionType::LH:
            codegen.generateInvoke(ExecutorLH, instr_offset);
            return;
        case InstructionType::LW:
            codegen.generateInvoke(ExecutorLW, instr_offset);
            return;
        case InstructionType::LD:
            codegen.generateInvoke(ExecutorLD, instr_offset);
            return;
        case InstructionType::LBU:
            codegen.generateInvoke(ExecutorLBU, instr_offset);
            return;
        case InstructionType::LHU:
            codegen.generateInvoke(ExecutorLHU, instr_offset);
            return;
        case InstructionType::LWU:
            codegen.generateInvoke(ExecutorLWU, instr_offset);
            return;
        case InstructionType::SB:
            codegen.generateInvoke(ExecutorSB, instr_offset);
            return;
        case InstructionType::SH:
            codegen.generateInvoke(ExecutorSH, instr_offset);
            return;
        case InstructionType::SW:
            codegen.generateInvoke(ExecutorSW, instr_offset);
            return;
        case InstructionType::SD:
            codegen.generateInvoke(ExecutorSD, instr_offset);
            return;
        case InstructionType::ADDI:
            codegen.generateInvoke(ExecutorADDI, instr_offset);
            return;
        case InstructionType::SLLI:
            codegen.generateInvoke(ExecutorSLLI, instr_offset);
            return;
        case InstructionType::SLTI:
            codegen.generateInvoke(ExecutorSLTI, instr_offset);
            return;
        case InstructionType::SLTIU:
            codegen.generateInvoke(ExecutorSLTIU, instr_offset);
            return;
        case InstructionType::XORI:
            codegen.generateInvoke(ExecutorXORI, instr_offset);
            return;
        case InstructionType::SRLI:
            codegen.generateInvoke(ExecutorSRLI, instr_offset);
            return;
        case InstructionType::SRAI:
            codegen.generateInvoke(ExecutorSRAI, instr_offset);
            return;
        case InstructionType::ORI:
            codegen.generateInvoke(ExecutorORI, instr_offset);
            return;
        case InstructionType::ANDI:
            codegen.generateInvoke(ExecutorANDI, instr_offset);
            return;
        case InstructionType::ADDIW:
            codegen.generateInvoke(ExecutorADDIW, instr_offset);
            return;
        case InstructionType::SLLIW:
            codegen.generateInvoke(ExecutorSLLIW, instr_offset);
            return;
        case InstructionType::SRLIW:
            codegen.generateInvoke(ExecutorSRLIW, instr_offset);
            return;
        case InstructionType::SRAIW:
            codegen.generateInvoke(ExecutorSRAIW, instr_offset);
            return;
        case InstructionType::ADD:
            codegen.generateInvoke(ExecutorADD, instr_offset);
            return;
        case InstructionType::SLL:
            codegen.generateInvoke(ExecutorSLL, instr_offset);
            return;
        case InstructionType::SLT:
            codegen.generateInvoke(ExecutorSLT, instr_offset);
            return;
        case InstructionType::SLTU:
            codegen.generateInvoke(ExecutorSLTU, instr_offset);
            return;
        case InstructionType::XOR:
            codegen.generateInvoke(ExecutorXOR, instr_offset);
            return;
        case InstructionType::SRL:
            codegen.generateInvoke(ExecutorSRL, instr_offset);
            return;
        case InstructionType::OR:
            codegen.generateInvoke(ExecutorOR, instr_offset);
            return;
        case InstructionType::AND:
            codegen.generateInvoke(ExecutorAND, instr_offset);
            return;
        case InstructionType::SUB:
            codegen.generateInvoke(ExecutorSUB, instr_offset);
            return;
        case InstructionType::SRA:
            codegen.generateInvoke(ExecutorSRA, instr_offset);
            return;
        case InstructionType::ADDW:
            codegen.generateInvoke(ExecutorADDW, instr_offset);
            return;
        case InstructionType::SUBW:
            codegen.generateInvoke(ExecutorSUBW, instr_offset);
            return;
        case InstructionType::SLLW:
            codegen.generateInvoke(ExecutorSLLW, instr_offset);
            return;
        case InstructionType::SRLW:
            codegen.generateInvoke(ExecutorSRLW, instr_offset);
            return;
        case InstructionType::SRAW:
            codegen.generateInvoke(ExecutorSRAW, instr_offset);
            return;
        case InstructionType::FENCE:
            codegen.generateInvoke(ExecutorFENCE, instr_offset);
            return;
        case InstructionType::ECALL:
            codegen.generateInvoke(ExecutorECALL, instr_offset);
            return;
        case InstructionType::EBREAK:
            codegen.generateInvoke(ExecutorEBREAK, instr_offset);
            return;
        case InstructionType::MUL:
            codegen.generateInvoke(ExecutorMUL, instr_offset);
            return;
        case InstructionType::MULH:
            codegen.generateInvoke(ExecutorMULH, instr_offset);
            return;
        case InstructionType::MULHSU:
            codegen.generateInvoke(ExecutorMULHSU, instr_offset);
            return;
        case InstructionType::MULHU:
            codegen.generateInvoke(ExecutorMULHU, instr_offset);
            return;
        case InstructionType::DIV:
            codegen.generateInvoke(ExecutorDIV, instr_offset);
            return;
        case InstructionType::DIVU:
            codegen.generateInvoke(ExecutorDIVU, instr_offset);
            return;
        case InstructionType::REM:
            codegen.generateInvoke(ExecutorREM, instr_offset);
            return;
        case InstructionType::REMU:
            codegen.generateInvoke(ExecutorREMU, instr_offset);
            return;
        case InstructionType::MULW:
            codegen.generateInvoke(ExecutorMULW, instr_offset);
            return;
        case InstructionType::DIVW:
            codegen.generateInvoke(ExecutorDIVW, instr_offset);
            return;
        case InstructionType::DIVUW:
            codegen.generateInvoke(ExecutorDIVUW, instr_offset);
            return;
        case InstructionType::REMW:
            codegen.generateInvoke(ExecutorREMW, instr_offset);
            return;
        case InstructionType::REMUW:
            codegen.generateInvoke(ExecutorREMUW, instr_offset);
            return;
        default:
            UNREACHABLE();
    }

    UNREACHABLE();
}

}  // namespace RISCV::compiler
