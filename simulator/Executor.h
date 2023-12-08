#ifndef EXECUTOR_H
#define EXECUTOR_H

namespace RISCV {
class Hart;
class DecodedInstruction;

using Executor = void (*)(Hart *, const DecodedInstruction &);
}  // namespace RISCV

#endif  // EXECUTOR_H
