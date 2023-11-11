#ifndef INCLUDE_HART_H
#define INCLUDE_HART_H

#include <array>

#include "BasicBlock.h"
#include "Cache.h"
#include "Common.h"
#include "Decoder.h"
#include "MMU.h"

namespace RISCV {

class Hart final {
public:
    static constexpr size_t BB_CACHE_CAPACITY = 1024;

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

    BasicBlock fetchBasicBlock();
    const BasicBlock& getBasicBlock();
    void executeBasicBlock(const BasicBlock& bb);
    void execute(const DecodedInstruction& decInstr);

    Hart();

    const memory::MMU& getTranslator() const {
        return mmu_;
    }

    inline memory::PhysAddr getPhysAddrR(const memory::VirtAddr vaddr) {
        memory::PhysAddr paddr;

        // Try TLB
        const uint64_t vpn = getPartialBits<12, 63>(vaddr);
        auto tlbEntry = tlb_.findR(vpn);
        if (tlbEntry != std::nullopt) {
            // TLB hit
            paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
            paddr += memory::getPageOffset(vaddr);
        } else {
            // TLB miss, translate address in usual way
            paddr = mmu_.getPhysAddrR(vaddr);
            tlb_.insertR(vpn, memory::getPageNumber(paddr));
        }

        return paddr;
    }

    inline memory::PhysAddr getPhysAddrW(const memory::VirtAddr vaddr) {
        memory::PhysAddr paddr;

        // Try TLB
        const uint64_t vpn = getPartialBits<12, 63>(vaddr);
        auto tlbEntry = tlb_.findW(vpn);
        if (tlbEntry != std::nullopt) {
            // TLB hit
            paddr = (*tlbEntry) * memory::PAGE_BYTESIZE;
            paddr += memory::getPageOffset(vaddr);
        } else {
            // TLB miss, translate address in usual way
            paddr = mmu_.getPhysAddrW(vaddr);
            tlb_.insertW(vpn, memory::getPageNumber(paddr));
        }

        return paddr;
    }

private:
    EncodedInstruction fetch();
    DecodedInstruction decode(const EncodedInstruction encInstr) const;

    memory::VirtAddr pc_;
    std::array<RegValue, RegisterType::REGISTER_COUNT> regs_ = {};
    std::array<RegValue, CSR_COUNT> csrRegs_ = {};

    memory::MMU mmu_;
    memory::TLB tlb_;

    LRUCache<BB_CACHE_CAPACITY, memory::VirtAddr, BasicBlock> bbCache_;

    Decoder decoder_;
};

}  // namespace RISCV

#endif  // INCLUDE_HART_H
