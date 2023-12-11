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
            codegen.generateLUI(instr);
            return;
        case InstructionType::AUIPC:
            codegen.generateAUIPC(instr);
            return;
        case InstructionType::JAL:
            codegen.generateJAL(instr);
            return;
        case InstructionType::JALR:
            codegen.generateJALR(instr);
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
            codegen.generateADDI(instr);
            return;
        case InstructionType::SLLI:
            codegen.generateSLLI(instr);
            return;
        case InstructionType::SLTI:
            codegen.generateSLTI(instr);
            return;
        case InstructionType::SLTIU:
            codegen.generateSLTIU(instr);
            return;
        case InstructionType::XORI:
            codegen.generateXORI(instr);
            return;
        case InstructionType::SRLI:
            codegen.generateSRLI(instr);
            return;
        case InstructionType::SRAI:
            codegen.generateSRAI(instr);
            return;
        case InstructionType::ORI:
            codegen.generateORI(instr);
            return;
        case InstructionType::ANDI:
            codegen.generateANDI(instr);
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
            codegen.generateADD(instr);
            return;
        case InstructionType::SLL:
            codegen.generateSLL(instr);
            return;
        case InstructionType::SLT:
            codegen.generateSLT(instr);
            return;
        case InstructionType::SLTU:
            codegen.generateSLTU(instr);
            return;
        case InstructionType::XOR:
            codegen.generateXOR(instr);
            return;
        case InstructionType::SRL:
            codegen.generateSRL(instr);
            return;
        case InstructionType::OR:
            codegen.generateOR(instr);
            return;
        case InstructionType::AND:
            codegen.generateAND(instr);
            return;
        case InstructionType::SUB:
            codegen.generateSUB(instr);
            return;
        case InstructionType::SRA:
            codegen.generateSRA(instr);
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
