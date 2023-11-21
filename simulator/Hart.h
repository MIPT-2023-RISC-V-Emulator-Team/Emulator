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

    using BBCache = LRUCache<BB_CACHE_CAPACITY, BasicBlock::Entrypoint, BasicBlock>;

    Hart();
    ~Hart();

    RegValue getReg(const RegisterType id) const {
        return regs_[id];
    }

    RegValue getCSRReg(const uint32_t id) const {
        return csrRegs_[id];
    }

    void setReg(const RegisterType id, const RegValue val) {
        regs_[id] = val;
        regs_[RegisterType::ZERO] = 0;
    }

    void incrementPC() {
        pc_ += INSTRUCTION_BYTESIZE;
    }

    memory::VirtAddr getPC() const {
        return pc_;
    }

    void setPC(memory::VirtAddr newPC) {
        pc_ = newPC;
    }

    BasicBlock& getBasicBlock();
    void executeBasicBlock(BasicBlock& bb);

    ALWAYS_INLINE auto cacheBasicBlock(BasicBlock::Entrypoint entrypoint, BasicBlock bb) {
        // TODO(panferovi): find way to remove lock
        std::lock_guard holder(bb_cache_lock_);
        return bbCache_.insert(entrypoint, std::move(bb));
    }

    void setBBEntry(BasicBlock::Entrypoint entrypoint, BasicBlock::CompiledEntry entry) {
        std::lock_guard holder(bb_cache_lock_);
        auto bb = bbCache_.find(entrypoint);

        if (UNLIKELY(bb == std::nullopt)) {
            return;
        }

        auto& bbRef = bb->get();
        bbRef.setCompiledEntry(entry);
        // TODO(all): implement codegen and uncomment
        // bbRef.setCompilationStatus(CompilationStatus::COMPILED, std::memory_order_release);
    }

    const memory::MMU& getTranslator() const {
        return mmu_;
    }

    template <memory::MemoryType type>
    inline memory::PhysAddr getPhysAddr(const memory::VirtAddr vaddr) {
        memory::PhysAddr paddr;

        // Try TLB
        const uint64_t vpn = getPartialBits<12, 63>(vaddr);
        auto tlbEntry = tlb_.find<type>(vpn);
        if (tlbEntry != std::nullopt) {
            // TLB hit
            paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
            paddr += memory::getPageOffset(vaddr);
        } else {
            // TLB miss, translate address in usual way
            paddr = mmu_.getPhysAddr<type>(vaddr);
            tlb_.insert<type>(vpn, memory::getPageNumber(paddr));
        }

        return paddr;
    }

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
    BBCache bbCache_;

    Decoder decoder_;
    Dispatcher dispatcher_;
    compiler::Compiler compiler_;
};

}  // namespace RISCV

#endif  // INCLUDE_HART_H
