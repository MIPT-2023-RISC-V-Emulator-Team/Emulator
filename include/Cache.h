#ifndef CACHE_H
#define CACHE_H

#include <functional>
#include <list>
#include <optional>
#include <unordered_map>

#include "macros.h"

namespace RISCV {

template <size_t CAPACITY, typename keyType, typename valType, bool byRef = true>
class LRUCache {
public:
    using Iter = typename std::list<std::pair<keyType, valType>>::const_iterator;
    using CRef = typename std::reference_wrapper<const valType>;

    using RetType = std::conditional_t<byRef, CRef, const valType>;

    std::optional<RetType> find(const keyType key) const {
        auto item = selector_.find(key);
        if (item == selector_.cend()) {
            return std::nullopt;
        }

        if constexpr (byRef) {
            return std::cref(item->second->second);
        } else {
            return item->second->second;
        }
    }

    RetType insert(const keyType key, const valType val) {
        ASSERT(selector_.find(key) == selector_.cend());
        if (storage_.size() == CAPACITY) {
            auto& last = storage_.back();
            selector_.erase(last.first);
            storage_.pop_back();
        }
        ASSERT(storage_.size() < CAPACITY);
        const auto& inserted = storage_.emplace_front(key, std::move(val));
        selector_.emplace(key, storage_.cbegin());

        if constexpr (byRef) {
            return std::cref(inserted.second);
        } else {
            return inserted.second;
        }
    }

private:
    std::list<std::pair<keyType, valType>> storage_;
    std::unordered_map<keyType, Iter> selector_;
};

}  // namespace RISCV

#endif  // CACHE_H
