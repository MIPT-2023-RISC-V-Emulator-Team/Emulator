#ifndef INCLUDE_HART_H
#define INCLUDE_HART_H

#include <array>
#include <mutex>

#include "compiler/Compiler.h"
#include "simulator/BasicBlock.h"
#include "simulator/Cache.h"
#include "simulator/Common.h"
#include "simulator/Decoder.h"
#include "simulator/Dispatcher.h"
#include "simulator/memory/MMU.h"

namespace RISCV {

class Hart final {
public:
    static constexpr size_t BB_CACHE_CAPACITY = 1024;

    Hart();
    ~Hart();

    ALWAYS_INLINE RegValue getReg(const RegisterType id) const {
        return regs_[id];
    }

    ALWAYS_INLINE RegValue getCSRReg(const uint32_t id) const {
        return csrRegs_[id];
    }

    ALWAYS_INLINE void setReg(const RegisterType id, const RegValue val) {
        regs_[id] = val;
        regs_[RegisterType::ZERO] = 0;
    }

    ALWAYS_INLINE void incrementPC() {
        pc_ += INSTRUCTION_BYTESIZE;
    }

    ALWAYS_INLINE memory::VirtAddr getPC() const {
        return pc_;
    }

    ALWAYS_INLINE void setPC(memory::VirtAddr newPC) {
        pc_ = newPC;
    }

    void executeBasicBlock(BasicBlock &bb);

    ALWAYS_INLINE auto cacheBasicBlock(BasicBlock::Entrypoint entrypoint, BasicBlock bb) {
        std::lock_guard holder(bb_cache_lock_);
        return bbCache_.insert(entrypoint, std::move(bb));
    }

    ALWAYS_INLINE BasicBlock &getBasicBlock() {
        auto bb = bbCache_.find(pc_);
        if (LIKELY(bb != std::nullopt)) {
            return *bb;
        }
        auto newBb = fetchBasicBlock();
        auto bbRef = cacheBasicBlock(pc_, std::move(newBb));
        return bbRef;
    }

    void setBBEntry(BasicBlock::Entrypoint entrypoint, BasicBlock::CompiledEntry entry);

    ALWAYS_INLINE const memory::MMU &getTranslator() const {
        return mmu_;
    }

    template <memory::MemoryType type>
    ALWAYS_INLINE memory::PhysAddr getPhysAddr(const memory::VirtAddr vaddr) {
        memory::PhysAddr paddr;

        // Try TLB
        const uint64_t vpn = memory::getPageNumber(vaddr);
        auto tlbEntry = tlb_.find<type>(vpn);
        if (LIKELY(tlbEntry != std::nullopt)) {
            // TLB hit
            paddr = *tlbEntry;
            paddr += memory::getPageOffset(vaddr);
        } else {
            // TLB miss, translate address in usual way
            paddr = mmu_.getPhysAddr<type>(vaddr);
            tlb_.insert<type>(vpn, memory::getPageNumberUnshifted(paddr));
        }

        return paddr;
    }

    static size_t getOffsetToRegs();
    static size_t getOffsetToPc();

private:
    BasicBlock fetchBasicBlock();
    EncodedInstruction fetch();
    DecodedInstruction decode(const EncodedInstruction encInstr) const;

    memory::VirtAddr pc_;
    std::array<RegValue, RegisterType::REGISTER_COUNT> regs_ = {};
    std::array<RegValue, CSR_COUNT> csrRegs_ = {};

    memory::MMU mmu_;
    memory::TLB tlb_;

    std::mutex bb_cache_lock_;
    BBCache<BB_CACHE_CAPACITY> bbCache_;

    Decoder decoder_;
    Dispatcher dispatcher_;
    compiler::Compiler compiler_;
};

}  // namespace RISCV

#endif  // INCLUDE_HART_H
