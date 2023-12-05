#ifndef INCLUDE_DISPATCHER_H
#define INCLUDE_DISPATCHER_H

#include "simulator/BasicBlock.h"

namespace RISCV {

class Hart;

class Dispatcher {
public:
    Dispatcher(Hart *hart) : hart_(hart) {}

    void dispatchExecute(BasicBlock::BodyEntry instr_iter);

private:
    Hart *hart_;
};

}  // namespace RISCV

#endif  // INCLUDE_DISPATCHER_H
