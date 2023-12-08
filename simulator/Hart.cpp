#include "simulator/Hart.h"

#include <iostream>

#include "compiler/Compiler.h"
#include "utils/macros.h"

namespace RISCV {

using namespace memory;

BasicBlock Hart::fetchBasicBlock() {
    EncodedInstruction encInstr[BasicBlock::MAX_SIZE];
    std::vector<DecodedInstruction> bbBody;
    bbBody.reserve(BasicBlock::MAX_SIZE);

    PhysicalMemory &pmem = getPhysicalMemory();
    PhysAddr paddr = getPhysAddr<memory::MemoryType::IMem>(pc_);

    constexpr const size_t maxBasicBlockBytesize = INSTRUCTION_BYTESIZE * BasicBlock::MAX_SIZE;
    constexpr const uint32_t maxOffsetForFullBB = PAGE_BYTESIZE - maxBasicBlockBytesize;

    const uint32_t pageOffset = getPageOffset(paddr);
    size_t readBytesize = maxBasicBlockBytesize;
    size_t readInstructions = BasicBlock::MAX_SIZE;

    if (UNLIKELY(pageOffset > maxOffsetForFullBB)) {
        // This is very rare
        readBytesize = PAGE_BYTESIZE - pageOffset;
        readInstructions = readBytesize / INSTRUCTION_BYTESIZE;
    }

    pmem.read(paddr, readBytesize, encInstr);
    for (size_t i = 0; i < readInstructions; ++i) {
        auto &decInstr = bbBody.emplace_back(decode(encInstr[i]));
        if (UNLIKELY(decInstr.isJumpInstruction())) {
            break;
        }
    }

    bbBody.emplace_back(DecodedInstruction{.type = BASIC_BLOCK_END});

    return BasicBlock(std::move(bbBody), pc_);
}

void Hart::executeBasicBlock(BasicBlock &bb) {
    auto isNotCompiled = compiler_.decrementHotnessCounter(bb);
    if (UNLIKELY(isNotCompiled)) {
        dispatcher_.dispatchExecute(bb.getBodyEntry());
        return;
    }
    bb.executeCompiled(this);
}

DecodedInstruction Hart::decode(const EncodedInstruction encInstr) const {
    return decoder_.decodeInstruction(encInstr);
}

Hart::Hart() : dispatcher_(this), compiler_(this) {
    PhysicalMemory &pmem = getPhysicalMemory();

    /*
     *  Initialize CSR
     */

    // Sv48 mode
    const TranslationMode satpMode = TranslationMode::TRANSLATION_MODE_SV48;
    // Make address space identifier 0
    const uint64_t satpAsid = 0;
    // Root table ppn
    const uint64_t satpPPN = pmem.getEmptyPageNumber();

    csrRegs_[CSR_SATP_INDEX] = makePartialBits<60, 63, uint64_t>(satpMode) | makePartialBits<44, 59>(satpAsid) |
                               makePartialBits<0, 43>(satpPPN);

    mmu_.setSATPReg(csrRegs_[CSR_SATP_INDEX]);
    pmem.allocatePage(satpPPN);

    compiler_.InitializeWorker();
}

Hart::~Hart() {
    compiler_.FinalizeWorker();
}

void Hart::setBBEntry(BasicBlock::Entrypoint entrypoint, BasicBlock::CompiledEntry entry) {
    std::lock_guard holder(bb_cache_lock_);
    auto bb = bbCache_.find(entrypoint);

    if (UNLIKELY(bb == std::nullopt)) {
        return;
    }

    auto &bbRef = bb->get();
    bbRef.setCompiledEntry(entry);
    bbRef.setCompilationStatus(CompilationStatus::COMPILED, std::memory_order_seq_cst);
}

size_t Hart::getOffsetToRegs() {
    return MEMBER_OFFSET(Hart, regs_);
}

size_t Hart::getOffsetToPc() {
    return MEMBER_OFFSET(Hart, pc_);
}

}  // namespace RISCV
