#ifndef DECODER_H
#define DECODER_H

#include "Common.h"

namespace RISCV {

class DecodedInstruction;

class Decoder {
public:
    DecodedInstruction decodeInstruction(const EncodedInstruction encInstr) const;
};

}  // namespace RISCV

#endif  // DECODER_H
