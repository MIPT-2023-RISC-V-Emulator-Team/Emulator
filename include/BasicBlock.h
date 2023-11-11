#ifndef INCLUDE_BASIC_BLOCK_H
#define INCLUDE_BASIC_BLOCK_H

#include <vector>

#include "Common.h"
#include "macros.h"

namespace RISCV {

class Hart;

class BasicBlock {
public:
    using Body = std::vector<DecodedInstruction>;

    // Fastest
    static constexpr size_t MAX_SIZE = 9;

    BasicBlock(Body body) : body_(std::move(body)) {}

    size_t getSize() const;
    void execute(Hart* hart) const;

private:
    Body body_;
};

}  // namespace RISCV

#endif  // INCLUDE_BASIC_BLOCK_H
