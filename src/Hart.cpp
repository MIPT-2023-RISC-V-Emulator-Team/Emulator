#include <iostream>

#include "Hart.h"
#include "Executor.h"

namespace RISCV {

const BasicBlock &Hart::getBasicBlock() {
    auto bb = bbCache_.find(pc_);
    if (bb != std::nullopt) {
        return *bb;
    }
    auto newBb = createBasicBlock();
    const auto &bbRef = bbCache_.insert(std::move(newBb));
    return bbRef;
}

BasicBlock Hart::createBasicBlock() {
    std::vector<DecodedInstruction> bbBody;
    auto entrypoint = pc_;
    while (bbBody.size() < BasicBlock::MAX_SIZE) {
        auto encInstr = fetch();
        auto &decInstr = bbBody.emplace_back(decode(encInstr));
        incrementPC();
        if (decInstr.isJumpInstruction()) {
            break;
        }
    }
    pc_ = entrypoint;
    return BasicBlock(entrypoint, std::move(bbBody));
}

void Hart::executeBasicBlock(const BasicBlock &bb) {
    bb.execute(this);
}

EncodedInstruction Hart::fetch() {
    EncodedInstruction encInstr;
    memory::PhysAddr paddr = mmu_.getPhysAddr(pc_, memory::MemoryRequestBits::R | memory::MemoryRequestBits::X);

    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();
    pmem.read(paddr, INSTRUCTION_BYTESIZE, &encInstr);

    return encInstr;
}

DecodedInstruction Hart::decode(const EncodedInstruction encInstr) const {
    return decoder_.decodeInstruction(encInstr);
}

void Hart::execute(const DecodedInstruction& decInstr) {
    decInstr.exec(this, decInstr);
}

Hart::Hart() {
    memory::PhysicalMemory& pmem = memory::getPhysicalMemory();

    /*
     *  Initialize CSR
     */

    // Sv48 mode
    const uint64_t satpMode = CSR_SATP_MODE_SV48;
    // Make address space identifier 0
    const uint64_t satpAsid = 0;
    // Root table ppn
    const uint64_t satpPPN  = pmem.getEmptyPageNumber();

    csrRegs_[CSR_SATP_INDEX] =
        makePartialBits<60, 63, uint64_t>(satpMode)
        | makePartialBits<44, 59, uint64_t>(satpAsid)
        | makePartialBits<0, 43, uint64_t>(satpPPN);

    mmu_.setSATPReg(&csrRegs_[CSR_SATP_INDEX]);
    pmem.allocatePage({satpPPN, 0});

    /*
     *  Initialize stack
     */

    regs_[RegisterType::SP] = memory::DEFAULT_STACK_ADDRESS;

    memory::VirtAddr stackVAddr = regs_[RegisterType::SP];
    constexpr const size_t stackPages = memory::STACK_BYTESIZE / memory::PAGE_BYTESIZE;
    for (size_t i = 0; i < stackPages; ++i) {
        memory::PhysAddr stackPAddr = mmu_.getPhysAddrWithAllocation(stackVAddr);
        pmem.allocatePage(stackPAddr);
        stackVAddr -= memory::PAGE_BYTESIZE;
    }
}


}  // namespace RISCV
