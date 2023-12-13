#ifndef INCLUDE_HOOKER_H
#define INCLUDE_HOOKER_H

#include <array>
#include <map>

#include "simulator/BasicBlock.h"
#include "simulator/Cache.h"
#include "simulator/Common.h"
#include "simulator/Decoder.h"

namespace RISCV {

class Hooker final {
public:
    Hooker();
    ~Hooker();

    void capturedInstr(EncodedInstruction &instr);

private:
    enum EVENTS { INSTR_EXECUTED };

    std::multimap<EncodedInstruction, std::string> hooks_;
};

}  // namespace RISCV

#endif  // INCLUDE_HOOKER_H
