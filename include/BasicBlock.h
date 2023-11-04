#ifndef INCLUDE_BASIC_BLOCK_H
#define INCLUDE_BASIC_BLOCK_H

#include <vector>
#include <list>
#include <unordered_map>
#include <optional>
#include <functional>

#include "Common.h"
#include "macros.h"

namespace RISCV {

class Hart;

class BasicBlock {
public:
    using Entrypoint = uint64_t;
    using Body = std::vector<DecodedInstruction>;

    static constexpr size_t MAX_SIZE = 8;

    BasicBlock(Entrypoint entry, Body body)
        : entrypoint_(entry), body_(std::move(body)) {}

    Entrypoint getEntry() const;
    size_t getSize() const;
    void execute(Hart *hart) const;

private:
    Body body_;
    Entrypoint entrypoint_;
};

template <size_t CAPACITY>
class BBCache {
public:
    using BBIter = std::list<BasicBlock>::const_iterator;
    using BBCRef = std::reference_wrapper<const BasicBlock>;
    using BBEntrypoint = BasicBlock::Entrypoint;

    std::optional<BBCRef> find(BBEntrypoint entry) {
        auto bbItem = selector_.find(entry);
        if (bbItem == selector_.cend()) {
            return std::nullopt;
        }
        bbStorage_.emplace_front(std::move(*bbItem->second));
        bbStorage_.erase(bbItem->second);
        bbItem->second = bbStorage_.cbegin();
        return std::cref(*bbItem->second);
    }

    BBCRef insert(BasicBlock bb) {
        ASSERT(selector_.find(bb.getEntry()) == selector_.cend());
        if (bbStorage_.size() == CAPACITY) {
            auto &lastBb = bbStorage_.back();
            selector_.erase(lastBb.getEntry());
            bbStorage_.pop_back();
        }
        ASSERT(bbStorage_.size() < CAPACITY);
        const auto &insertedBb = bbStorage_.emplace_front(std::move(bb));
        selector_.emplace(insertedBb.getEntry(), bbStorage_.cbegin());
        return insertedBb;
    }

private:
    std::list<BasicBlock> bbStorage_;
    std::unordered_map<BBEntrypoint, BBIter> selector_;
};

}  // RISCV

#endif  // INCLUDE_BASIC_BLOCK_H
