#include "Hart.h"

#include <iostream>

#include "Executor.h"

namespace RISCV {

EncodedInstruction Hart::fetch() {
    EncodedInstruction encInstr;
    if (!mmu_.load32(pc_, &encInstr)) {
        printf("Could not handle page fault\n");
        exit(EXIT_FAILURE);
    }
    return encInstr;
}

DecodedInstruction Hart::decode(const EncodedInstruction encInstr) const {
    return decoder_.decodeInstruction(encInstr);
}

void Hart::execute(const DecodedInstruction& decInstr) {
    decInstr.exec(this, decInstr);
}

}  // namespace RISCV
