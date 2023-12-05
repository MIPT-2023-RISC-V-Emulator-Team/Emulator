#include "simulator/Hart.h"

#include <iostream>

#include "compiler/Compiler.h"
#include "utils/macros.h"

namespace RISCV {

using namespace memory;

BasicBlock& Hart::getBasicBlock() {
    auto bb = bbCache_.find(pc_);
    if (LIKELY(bb != std::nullopt)) {
        return *bb;
    }
    auto newBb = fetchBasicBlock();
    auto bbRef = cacheBasicBlock(pc_, std::move(newBb));
    return bbRef;
}

BasicBlock Hart::fetchBasicBlock() {
    EncodedInstruction encInstr[BasicBlock::MAX_SIZE];
    std::vector<DecodedInstruction> bbBody;
    bbBody.reserve(BasicBlock::MAX_SIZE);

    constexpr const size_t maxBasicBlockBytesize = INSTRUCTION_BYTESIZE * BasicBlock::MAX_SIZE;

    PhysicalMemory& pmem = getPhysicalMemory();
    PhysAddr paddr = getPhysAddr<memory::MemoryType::IMem>(pc_);

    const uint32_t pageOffset = getPageOffset(paddr);
    size_t readBytesize = PAGE_BYTESIZE - pageOffset;
    size_t readInstructions = BasicBlock::MAX_SIZE;

    if (LIKELY(readBytesize >= maxBasicBlockBytesize)) {
        // Hooooot
        readBytesize = maxBasicBlockBytesize;
    } else {
        // This is very rare
        readInstructions = readBytesize / INSTRUCTION_BYTESIZE;
    }

    pmem.read(paddr, readBytesize, encInstr);
    for (size_t i = 0; i < readInstructions; ++i) {
        auto& decInstr = bbBody.emplace_back(decode(encInstr[i]));
        if (UNLIKELY(decInstr.isJumpInstruction())) {
            break;
        }
    }

    bbBody.emplace_back(DecodedInstruction{.type = BASIC_BLOCK_END});

    return BasicBlock(std::move(bbBody), pc_);
}

void Hart::executeBasicBlock(BasicBlock& bb) {
    auto is_not_compiled = compiler_.decrementHotnessCounter(bb);
    if (is_not_compiled) {
        dispatcher_.dispatchExecute(bb.getBodyEntry());
        return;
    }
    bb.executeCompiled(this);
}

DecodedInstruction Hart::decode(const EncodedInstruction encInstr) const {
    return decoder_.decodeInstruction(encInstr);
}

Hart::Hart() : dispatcher_(this), compiler_(this) {
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
    pmem.allocatePage(satpPPN);

    compiler_.InitializeWorker();
}

Hart::~Hart() {
    compiler_.FinalizeWorker();
}

}  // namespace RISCV
