#include <iostream>

#include "Hart.h"
#include "Executor.h"

namespace RISCV {

const BasicBlock &Hart::getBasicBlock() {
    auto bb = bbCache_.find(pc_);
    if (bb != std::nullopt) {
        return *bb;
    }
    auto newBb = createBasicBlock();
    const auto &bbRef = bbCache_.insert(std::move(newBb));
    return bbRef;
}

BasicBlock Hart::createBasicBlock() {
    std::vector<DecodedInstruction> bbBody;
    auto entrypoint = pc_;
    while (bbBody.size() < BasicBlock::MAX_SIZE) {
        auto encInstr = fetch();
        auto &decInstr = bbBody.emplace_back(decode(encInstr));
        incrementPC();
        if (decInstr.isJumpInstruction()) {
            break;
        }
    }
    pc_ = entrypoint;
    return BasicBlock(entrypoint, std::move(bbBody));
}

void Hart::executeBasicBlock(const BasicBlock &bb) {
    bb.execute(this);
}

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
