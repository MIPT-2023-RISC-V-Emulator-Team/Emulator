#ifndef INCLUDE_BASIC_BLOCK_H
#define INCLUDE_BASIC_BLOCK_H

#include <atomic>
#include <vector>

#include "simulator/DecodedInstruction.h"
#include "utils/macros.h"

namespace RISCV {

class Hart;

enum class CompilationStatus : uint8_t { NOT_COMPILED, COMPILING, COMPILED };

class BasicBlock {
public:
    using Body = std::vector<DecodedInstruction>;
    using BodyEntry = Body::const_iterator;
    using Entrypoint = uint64_t;
    using CompiledEntry = void (*)(Hart *);

    // Fastest
    static constexpr size_t MAX_SIZE = 9;
    static constexpr uint32_t START_HOTNESS_COUNTER = 10;

    BasicBlock(Body body, Entrypoint entrypoint) : body_(std::move(body)), entrypoint_(entrypoint) {
        ASSERT(body_.back().type == BASIC_BLOCK_END);
    }

    BasicBlock(const BasicBlock &bb)
        : body_(bb.body_),
          entrypoint_(bb.entrypoint_),
          hotness_counter_(bb.hotness_counter_),
          compiled_entry_(bb.compiled_entry_),
          compilation_status_(bb.compilation_status_.load(std::memory_order_relaxed)) {}

    BasicBlock(BasicBlock &&bb)
        : body_(std::move(bb.body_)),
          entrypoint_(bb.entrypoint_),
          hotness_counter_(bb.hotness_counter_),
          compiled_entry_(bb.compiled_entry_),
          compilation_status_(bb.compilation_status_.load(std::memory_order_relaxed)) {}

    BasicBlock() = default;

    ~BasicBlock() = default;

    BasicBlock &operator=(BasicBlock &&bb) {
        body_ = std::move(bb.body_);
        entrypoint_ = std::move(bb.entrypoint_);
        hotness_counter_ = std::move(bb.hotness_counter_);
        compiled_entry_ = std::move(bb.compiled_entry_);
        compilation_status_ = std::move(bb.compilation_status_.load(std::memory_order_relaxed));
        return *this;
    }

    ALWAYS_INLINE size_t getSize() const {
        return body_.size();
    }

    ALWAYS_INLINE BodyEntry getBodyEntry() const {
        return body_.cbegin();
    }

    ALWAYS_INLINE Body getBody() const {
        return body_;
    }

    ALWAYS_INLINE Entrypoint getEntrypoint() const {
        return entrypoint_;
    }

    ALWAYS_INLINE void executeCompiled(Hart *hart) const {
        compiled_entry_(hart);
    }

    ALWAYS_INLINE CompilationStatus getCompilationStatus(std::memory_order memory_order) const {
        return compilation_status_.load(memory_order);
    }

    ALWAYS_INLINE void setCompilationStatus(CompilationStatus status, std::memory_order memory_order) {
        compilation_status_.store(status, memory_order);
    }

    ALWAYS_INLINE uint32_t decrementHotnessCounter() {
        return --hotness_counter_;
    }

    ALWAYS_INLINE void setCompiledEntry(CompiledEntry compiled_entry) {
        ASSERT(compiled_entry_ == nullptr);
        compiled_entry_ = compiled_entry;
    }

private:
    Body body_;
    Entrypoint entrypoint_;
    uint32_t hotness_counter_{START_HOTNESS_COUNTER};
    CompiledEntry compiled_entry_{nullptr};
    std::atomic<CompilationStatus> compilation_status_{CompilationStatus::NOT_COMPILED};
};

}  // namespace RISCV

#endif  // INCLUDE_BASIC_BLOCK_H
