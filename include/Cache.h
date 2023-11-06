#ifndef CACHE_H
#define CACHE_H


#include <list>
#include <unordered_map>
#include <optional>
#include <functional>


namespace RISCV {


template <size_t CAPACITY, typename keyType, typename valType, bool byRef = true>
class LRUCache {
public:

    using Iter = typename std::list<std::pair<keyType, valType>>::const_iterator;
    using CRef = typename std::reference_wrapper<const valType>;

    using RetType = std::conditional_t<byRef, CRef, const valType>;

    std::optional<RetType> find(const keyType key) const {
        if constexpr (byRef) {
            return findByRef(key);
        }
        else {
            return findByVal(key);
        }
    }

    RetType insert(const keyType key, const valType val) {
        if constexpr (byRef) {
            return insertByRef(key, val);
        }
        else {
            return insertByVal(key, val);
        }
    }

private:

    std::optional<CRef> findByRef(const keyType key) const {
        auto item = selector_.find(key);
        if (item == selector_.cend()) {
            return std::nullopt;
        }
        return std::cref(item->second->second);
    }

    CRef insertByRef(const keyType key, const valType val) {
        ASSERT(selector_.find(key) == selector_.cend());
        if (storage_.size() == CAPACITY) {
            auto &last = storage_.back();
            selector_.erase(last.first);
            storage_.pop_back();
        }
        ASSERT(storage_.size() < CAPACITY);
        const auto &inserted = storage_.emplace_front(key, std::move(val));
        selector_.emplace(key, storage_.cbegin());
        return std::cref(inserted.second);
    }


    std::optional<const valType> findByVal(const keyType key) const {
        auto item = selector_.find(key);
        if (item == selector_.cend()) {
            return std::nullopt;
        }
        return item->second->second;
    }

    valType insertByVal(const keyType key, const valType val) {
        ASSERT(selector_.find(key) == selector_.cend());
        if (storage_.size() == CAPACITY) {
            auto last = storage_.back();
            selector_.erase(last.first);
            storage_.pop_back();
        }
        ASSERT(storage_.size() < CAPACITY);
        const auto inserted = storage_.emplace_front(key, val);
        selector_.emplace(key, storage_.cbegin());
        return inserted.second;
    }


    std::list<std::pair<keyType, valType>> storage_;
    std::unordered_map<keyType, Iter> selector_;
};

}   // RISCV

#endif  // CACHE_H