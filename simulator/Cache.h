#ifndef CACHE_H
#define CACHE_H

#include <functional>
#include <list>
#include <optional>
#include <unordered_map>

#include "BasicBlock.h"
#include "utils/macros.h"

namespace RISCV {

template <size_t CAPACITY>
class BBCache {
public:
    using RetType = typename std::reference_wrapper<BasicBlock>;
    static constexpr const uint64_t checkBits = CAPACITY - 1;

    std::optional<RetType> find(const BasicBlock::Entrypoint pc) {
        std::pair<BasicBlock::Entrypoint, BasicBlock> &pc_bb = storage_[pc & checkBits];
        if (LIKELY(pc_bb.first == pc)) {
            return std::ref(pc_bb.second);
        }
        return std::nullopt;
    }

    RetType insert(const BasicBlock::Entrypoint pc, BasicBlock bb) {
        const size_t idx = pc & checkBits;
        storage_[idx].first = pc;
        storage_[idx].second = std::move(bb);
        return std::ref(storage_[idx].second);
    }

private:
    std::pair<BasicBlock::Entrypoint, BasicBlock> storage_[CAPACITY];
};

}  // namespace RISCV

#endif  // CACHE_H
