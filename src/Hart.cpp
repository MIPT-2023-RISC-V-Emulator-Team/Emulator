#include "Hart.h"

#include <iostream>

#include "Executor.h"

namespace RISCV {

void Hart::fetch(EncodedInstruction& encInstr) {
    if (!mmu_.load32(pc_, &encInstr)) {
        printf("Could not handle page fault\n");
        exit(EXIT_FAILURE);
    }
}

void Hart::decode(const EncodedInstruction encInstr, DecodedInstruction& decInstr) const {
    // Initialize instruction to be invalid so every return will
    // be either valid instruction or invalid one
    decInstr.type = InstructionType::INSTRUCTION_INVALID;
    decoder_.decodeInstruction(encInstr, decInstr);

    return;
}

void Hart::execute(const DecodedInstruction& decInstr) {
    decInstr.exec(this, decInstr);
}

}  // namespace RISCV
