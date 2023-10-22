#ifndef INCLUDE_HART_H
#define INCLUDE_HART_H

#include <array>

#include "Common.h"
#include "Decoder.h"
#include "BasicBlock.h"
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

    BasicBlock createBasicBlock();
    const BasicBlock &getBasicBlock();
    void executeBasicBlock(const BasicBlock &bb);
    void execute(const DecodedInstruction& decInstr);

    Hart();

    const memory::MMU& getTranslator() const {
        return mmu_;
    }

private:
    EncodedInstruction fetch();
    DecodedInstruction decode(const EncodedInstruction encInstr) const;

    memory::VirtAddr pc_;
    std::array<RegValue, RegisterType::REGISTER_COUNT> regs_ = {};
    std::array<RegValue, CSR_COUNT> csrRegs_ = {};
    memory::MMU mmu_;

    BBCache<BB_CACHE_CAPACITY> bbCache_;

    Decoder decoder_;
};

}  // namespace RISCV

#endif  // INCLUDE_HART_H
