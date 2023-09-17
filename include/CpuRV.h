#ifndef CPU_RV_H
#define CPU_RV_H

#include "Common.h"
#include "Memory.h"

namespace RISCV {

class CpuRV {

private:
    Register regs_[RegisterType::REGISTER_COUNT] = {0};
	uint64_t pc_;
    MMU mmu_;

public:
	void fetch(EncodedInstruction& encInstr);
	void decode(const EncodedInstruction& encInstr, DecodedInstruction& decInstr) const;
	void execute(const DecodedInstruction& decInstr);

    uint64_t getPC() const { return pc_; };
    void loadElfFile(const std::string& filename) { mmu_.loadElfFile(filename, &pc_); }

    CpuRV() { regs_[RegisterType::SP].value = mmu_.getStackAddress(); };
};

}


#endif // CPU_RV_H
