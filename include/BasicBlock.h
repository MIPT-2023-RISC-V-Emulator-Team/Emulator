#ifndef INCLUDE_BASIC_BLOCK_H
#define INCLUDE_BASIC_BLOCK_H

#include <vector>

#include "DecodedInstruction.h"
#include "macros.h"

namespace RISCV {

class BasicBlock {
public:
    using Body = std::vector<DecodedInstruction>;
    using EntryPoint = Body::const_iterator;

    // Fastest
    static constexpr size_t MAX_SIZE = 9;

    BasicBlock(Body body) : body_(std::move(body)) {
        ASSERT(body_.back().type == BASIC_BLOCK_END);
    }

    size_t getSize() const {
        return body_.size();
    }

    EntryPoint getEntryPoint() const {
        return body_.cbegin();
    }

private:
    Body body_;
};

}  // namespace RISCV

#endif  // INCLUDE_BASIC_BLOCK_H
