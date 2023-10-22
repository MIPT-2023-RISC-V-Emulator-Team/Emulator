#ifndef INCLUDE_HART_H
#define INCLUDE_HART_H

#include <array>

#include "Common.h"
#include "Decoder.h"
#include "Memory.h"

namespace RISCV {

class Hart {
public:
    RegValue getReg(const RegisterType id) const {
        return regs_[id];
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

    EncodedInstruction fetch();
    DecodedInstruction decode(const EncodedInstruction encInstr) const;
    void execute(const DecodedInstruction& decInstr);


    Hart() {
        regs_[RegisterType::SP] = memory::DEFAULT_STACK_ADDRESS;
    }

    // Temporary solution is to make MMU public. TODO: organize memory
    memory::MMU mmu_;

private:
    memory::VirtAddr pc_;
    std::array<RegValue, RegisterType::REGISTER_COUNT> regs_ = {};

    Decoder decoder_;
};

}  // namespace RISCV

#endif  // INCLUDE_HART_H
