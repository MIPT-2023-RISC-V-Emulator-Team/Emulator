#include "Hart.h"

#include <iostream>

#include "Executor.h"

namespace RISCV {

using namespace memory;

const BasicBlock& Hart::getBasicBlock() {
    auto bb = bbCache_.find(pc_);
    if (bb != std::nullopt) {
        return *bb;
    }
    auto newBb = fetchBasicBlock();
    const auto& bbRef = bbCache_.insert(pc_, std::move(newBb));
    return bbRef;
}

BasicBlock Hart::fetchBasicBlock() {
    EncodedInstruction encInstr[BasicBlock::MAX_SIZE];
    std::vector<DecodedInstruction> bbBody;

    constexpr const size_t maxBasicBlockBytesize = INSTRUCTION_BYTESIZE * BasicBlock::MAX_SIZE;

    PhysicalMemory& pmem = getPhysicalMemory();
    PhysAddr paddr;

    // Try TLB
    const uint64_t vpn = getPartialBits<12, 63>(pc_);
    auto tlbEntry = tlb_.findI(vpn);
    if (tlbEntry != std::nullopt) {
        // TLB hit
        paddr = (*tlbEntry) * PAGE_BYTESIZE;
        paddr += getPageOffset(pc_);
    } else {
        // TLB miss, translate address in usual way
        paddr = mmu_.getPhysAddrI(pc_);
        tlb_.insertI(vpn, getPageNumber(paddr));
    }

    const uint32_t pageOffset = getPageOffset(paddr);
    size_t readBytesize = PAGE_BYTESIZE - pageOffset;
    size_t readInstructions = BasicBlock::MAX_SIZE;

    if (readBytesize >= maxBasicBlockBytesize) {
        // Hooooot
        readBytesize = maxBasicBlockBytesize;
    } else {
        // This is very rare
        readInstructions = readBytesize / INSTRUCTION_BYTESIZE;
    }

    pmem.read(paddr, readBytesize, encInstr);
    for (size_t i = 0; i < readInstructions; ++i) {
        auto& decInstr = bbBody.emplace_back(decode(encInstr[i]));
        if (decInstr.isJumpInstruction()) {
            break;
        }
    }

    return BasicBlock(std::move(bbBody));
}

void Hart::executeBasicBlock(const BasicBlock& bb) {
    bb.execute(this);
}

DecodedInstruction Hart::decode(const EncodedInstruction encInstr) const {
    return decoder_.decodeInstruction(encInstr);
}

void Hart::execute(const DecodedInstruction& decInstr) {
    decInstr.exec(this, decInstr);
}

Hart::Hart() {
    PhysicalMemory& pmem = getPhysicalMemory();

    /*
     *  Initialize CSR
     */

    // Sv48 mode
    const TranslationMode satpMode = TranslationMode::TRANSLATION_MODE_SV48;
    // Make address space identifier 0
    const uint64_t satpAsid = 0;
    // Root table ppn
    const uint64_t satpPPN = pmem.getEmptyPageNumber();

    csrRegs_[CSR_SATP_INDEX] = makePartialBits<60, 63, uint64_t>(satpMode) |
                               makePartialBits<44, 59>(satpAsid) | makePartialBits<0, 43>(satpPPN);

    mmu_.setSATPReg(csrRegs_[CSR_SATP_INDEX]);
    pmem.allocatePage(satpPPN * PAGE_BYTESIZE);

    /*
     *  Initialize stack
     */

    regs_[RegisterType::SP] = DEFAULT_STACK_ADDRESS;

    VirtAddr stackVAddr = regs_[RegisterType::SP];
    constexpr const size_t stackPages = STACK_BYTESIZE / PAGE_BYTESIZE;
    for (size_t i = 0; i < stackPages; ++i) {
        PhysAddr stackPAddr = mmu_.getPhysAddrWithAllocation(stackVAddr);
        pmem.allocatePage(stackPAddr);
        stackVAddr -= PAGE_BYTESIZE;
    }
}

}  // namespace RISCV
