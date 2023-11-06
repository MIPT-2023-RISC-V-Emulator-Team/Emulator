#include "BasicBlock.h"
#include "Hart.h"

namespace RISCV {

size_t BasicBlock::getSize() const {
    return body_.size();
}

void BasicBlock::execute(Hart *hart) const {
    for (auto &&decInstr : body_) {
        hart->execute(decInstr);
    }
}

}  // namespace RISCV
