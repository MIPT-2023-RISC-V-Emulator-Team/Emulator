#include "Hooker.h"

namespace RISCV {

void Hooker::capturedInstr(EncodedInstruction &instr) {
  std::string exact_instr = "callee";
  hooks_.insert(instr, exact_instr);
}

} // namespace RISCV