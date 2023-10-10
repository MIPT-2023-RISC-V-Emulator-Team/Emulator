#include "Decoder.h"

namespace RISCV {

void Decoder::decodeInstruction(const EncodedInstruction encInstr,
                                DecodedInstruction& decInstr) const {
    const uint32_t opcode = encInstr & 0b1111111;

    switch (opcode) {
        case 0b0110111: {  // LUI
            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.imm = getPartialBits<31, 12>(encInstr);
            decInstr.immSignBitNum = 31;
            decInstr.exec = ExecutorLUI;

            decInstr.type = InstructionType::LUI;
            return;
        }
        case 0b0010111: {  // AUIPC
            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.imm = getPartialBits<31, 12>(encInstr);
            decInstr.immSignBitNum = 31;
            decInstr.exec = ExecutorAUIPC;

            decInstr.type = InstructionType::AUIPC;
            return;
        }
        case 0b1101111: {  // JAL
            decInstr.type = InstructionType::JAL;
            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));

            decInstr.imm =
                (getPartialBits<30, 21>(encInstr) >> 20) + (getPartialBits<20, 20>(encInstr) >> 9) +
                (getPartialBits<19, 12>(encInstr)) + (getPartialBits<31, 31>(encInstr) >> 11);

            decInstr.immSignBitNum = 20;
            decInstr.exec = ExecutorJAL;
            return;
        }
        case 0b1100111: {  // JALR
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            if (funct3 == 0b000) {
                decInstr.type = InstructionType::JALR;
                decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
                decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
                decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                decInstr.immSignBitNum = 11;
                decInstr.exec = ExecutorJALR;
            }
            return;
        }
        case 0b1100011: {  // BEQ or BNE or BLT or BGE or BLTU or BGEU
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            switch (funct3) {
                case 0b000: {  // BEQ
                    decInstr.type = InstructionType::BEQ;
                    decInstr.exec = ExecutorBEQ;
                    break;
                }
                case 0b001: {  // BNE
                    decInstr.type = InstructionType::BNE;
                    decInstr.exec = ExecutorBNE;
                    break;
                }
                case 0b100: {  // BLT
                    decInstr.type = InstructionType::BLT;
                    decInstr.exec = ExecutorBLT;
                    break;
                }
                case 0b101: {  // BGE
                    decInstr.type = InstructionType::BGE;
                    decInstr.exec = ExecutorBGE;
                    break;
                }
                case 0b110: {  // BLTU
                    decInstr.type = InstructionType::BLTU;
                    decInstr.exec = ExecutorBLTU;
                    break;
                }
                case 0b111: {  // BGEU
                    decInstr.type = InstructionType::BGEU;
                    decInstr.exec = ExecutorBGEU;
                    break;
                }
                default:
                    return;
            }

            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            decInstr.rs2 = static_cast<RegisterType>(getPartialBitsShifted<24, 20>(encInstr));

            decInstr.imm =
                (getPartialBits<11, 8>(encInstr) >> 7) + (getPartialBits<30, 25>(encInstr) >> 20) +
                (getPartialBits<7, 7>(encInstr) << 4) + (getPartialBits<31, 31>(encInstr) >> 19);

            decInstr.immSignBitNum = 12;
            return;
        }
        case 0b0000011: {  // LB or LH or LW or LD or LBU or LHU or LWU
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            switch (funct3) {
                case 0b000: {  // LB
                    decInstr.type = InstructionType::LB;
                    decInstr.exec = ExecutorLB;
                    break;
                }
                case 0b001: {  // LH
                    decInstr.type = InstructionType::LH;
                    decInstr.exec = ExecutorLH;
                    break;
                }
                case 0b010: {  // LW
                    decInstr.type = InstructionType::LW;
                    decInstr.exec = ExecutorLW;
                    break;
                }
                case 0b011: {  // LD
                    decInstr.type = InstructionType::LD;
                    decInstr.exec = ExecutorLD;
                    break;
                }
                case 0b100: {  // LBU
                    decInstr.type = InstructionType::LBU;
                    decInstr.exec = ExecutorLBU;
                    break;
                }
                case 0b101: {  // LHU
                    decInstr.type = InstructionType::LHU;
                    decInstr.exec = ExecutorLHU;
                    break;
                }
                case 0b110: {  // LWU
                    decInstr.type = InstructionType::LWU;
                    decInstr.exec = ExecutorLWU;
                    break;
                }
                default:
                    return;
            }

            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
            decInstr.immSignBitNum = 11;
            return;
        }
        case 0b0100011: {  // SB or SH or SW or SD
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            switch (funct3) {
                case 0b000: {  // SB
                    decInstr.type = InstructionType::SB;
                    decInstr.exec = ExecutorSB;
                    break;
                }
                case 0b001: {  // SH
                    decInstr.type = InstructionType::SH;
                    decInstr.exec = ExecutorSH;
                    break;
                }
                case 0b010: {  // SW
                    decInstr.type = InstructionType::SW;
                    decInstr.exec = ExecutorSW;
                    break;
                }
                case 0b011: {  // SD
                    decInstr.type = InstructionType::SD;
                    decInstr.exec = ExecutorSD;
                    break;
                }
                default:
                    return;
            }

            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            decInstr.rs2 = static_cast<RegisterType>(getPartialBitsShifted<24, 20>(encInstr));
            decInstr.imm =
                (getPartialBits<11, 7>(encInstr) >> 7) + (getPartialBits<31, 25>(encInstr) >> 20);
            decInstr.immSignBitNum = 11;
            return;
        }
        case 0b0010011: {  // ADDI or SLLI or SLTI or SLTIU or XORI or SRLI or SRAI or ORI or ANDI
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            const uint32_t funct7 = getPartialBitsShifted<31, 25>(encInstr);
            switch (funct3) {
                case 0b000: {  // ADDI
                    decInstr.type = InstructionType::ADDI;
                    decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                    decInstr.immSignBitNum = 11;
                    decInstr.exec = ExecutorADDI;
                    break;
                }
                case 0b001: {  // SLLI
                    if (funct7 == 0b0000000) {
                        decInstr.type = InstructionType::SLLI;
                        decInstr.shamt = getPartialBitsShifted<25, 20>(encInstr);
                        decInstr.exec = ExecutorSLLI;
                        break;
                    }
                    return;
                }
                case 0b010: {  // SLTI
                    decInstr.type = InstructionType::SLTI;
                    decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                    decInstr.immSignBitNum = 11;
                    decInstr.exec = ExecutorSLTI;
                    break;
                }
                case 0b011: {  // SLTIU
                    decInstr.type = InstructionType::SLTIU;
                    decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                    decInstr.immSignBitNum = 11;
                    decInstr.exec = ExecutorSLTIU;
                    break;
                }
                case 0b100: {  // XORI
                    decInstr.type = InstructionType::XORI;
                    decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                    decInstr.immSignBitNum = 11;
                    decInstr.exec = ExecutorXORI;
                    break;
                }
                case 0b101: {                   // SRLI or SRAI
                    if (funct7 == 0b0000000) {  // SRLI
                        decInstr.type = InstructionType::SRLI;
                        decInstr.shamt = getPartialBitsShifted<25, 20>(encInstr);
                        decInstr.exec = ExecutorSRLI;
                        break;
                    } else if (funct7 == 0b0100000) {  // SRAI
                        decInstr.type = InstructionType::SRAI;
                        decInstr.shamt = getPartialBitsShifted<25, 20>(encInstr);
                        decInstr.exec = ExecutorSRAI;
                        break;
                    }
                    return;
                }
                case 0b110: {  // ORI
                    decInstr.type = InstructionType::ORI;
                    decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                    decInstr.immSignBitNum = 11;
                    decInstr.exec = ExecutorORI;
                    break;
                }
                case 0b111: {  // ANDI
                    decInstr.type = InstructionType::ANDI;
                    decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                    decInstr.immSignBitNum = 11;
                    decInstr.exec = ExecutorANDI;
                    break;
                }
                default:
                    return;
            }

            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            return;
        }
        case 0b0110011: {  // ADD or SLL or SLT or SLTU or XOR or SRL or OR or AND or SUB or SRA
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            const uint32_t funct7 = getPartialBitsShifted<31, 25>(encInstr);
            if (funct7 == 0b0000000) {
                switch (funct3) {
                    case 0b000: {  // ADD
                        decInstr.type = InstructionType::ADD;
                        decInstr.exec = ExecutorADD;
                        break;
                    }
                    case 0b001: {  // SLL
                        decInstr.type = InstructionType::SLL;
                        decInstr.exec = ExecutorSLL;
                        break;
                    }
                    case 0b010: {  // SLT
                        decInstr.type = InstructionType::SLT;
                        decInstr.exec = ExecutorSLT;
                        break;
                    }
                    case 0b011: {  // SLTU
                        decInstr.type = InstructionType::SLTU;
                        decInstr.exec = ExecutorSLTU;
                        break;
                    }
                    case 0b100: {  // XOR
                        decInstr.type = InstructionType::XOR;
                        decInstr.exec = ExecutorXOR;
                        break;
                    }
                    case 0b101: {  // SRL
                        decInstr.type = InstructionType::SRL;
                        decInstr.exec = ExecutorSRL;
                        break;
                    }
                    case 0b110: {  // OR
                        decInstr.type = InstructionType::OR;
                        decInstr.exec = ExecutorOR;
                        break;
                    }
                    case 0b111: {  // AND
                        decInstr.type = InstructionType::AND;
                        decInstr.exec = ExecutorAND;
                        break;
                    }
                    default:
                        return;
                }
            } else if (funct7 == 0b0100000) {
                switch (funct3) {
                    case 0b000: {  // SUB
                        decInstr.type = InstructionType::SUB;
                        decInstr.exec = ExecutorSUB;
                        break;
                    }
                    case 0b101: {  // SRA
                        decInstr.type = InstructionType::SRA;
                        decInstr.exec = ExecutorSRA;
                        break;
                    }
                    default:
                        return;
                }
            } else {
                return;
            }

            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            decInstr.rs2 = static_cast<RegisterType>(getPartialBitsShifted<24, 20>(encInstr));
            return;
        }
        case 0b0001111: {  // FENCE
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            if (funct3 == 0b000) {
                decInstr.type = InstructionType::FENCE;

                decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
                decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
                decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                decInstr.immSignBitNum = 11;
                decInstr.exec = ExecutorFENCE;
            }
            return;
        }
        case 0b1110011: {  // ECALL or EBREAK
            const uint32_t funct12 = getPartialBitsShifted<31, 20>(encInstr);
            if (funct12 == 0b000000000000) {  // ECALL
                decInstr.type = InstructionType::ECALL;
                decInstr.exec = ExecutorECALL;
            } else if (funct12 == 0b000000000001) {  // EBREAK
                decInstr.type = InstructionType::EBREAK;
                decInstr.exec = ExecutorEBREAK;
            } else {
                return;
            }

            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            return;
        }
        case 0b0011011: {  // ADDIW or SLLIW or SRLIW or SRAIW
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            if (funct3 == 0b000) {  // ADDIW
                decInstr.type = InstructionType::ADDIW;
                decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
                decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
                decInstr.imm = getPartialBitsShifted<31, 20>(encInstr);
                decInstr.immSignBitNum = 11;
                decInstr.exec = ExecutorADDIW;
                return;
            }

            const uint32_t funct6 = getPartialBitsShifted<31, 26>(encInstr);
            if (funct6 == 0b000000) {
                switch (funct3) {
                    case 0b001: {  // SLLIW
                        decInstr.type = InstructionType::SLLIW;
                        decInstr.exec = ExecutorSLLIW;
                        break;
                    }
                    case 0b101: {  // SRLIW
                        decInstr.type = InstructionType::SRLIW;
                        decInstr.exec = ExecutorSRLIW;
                        break;
                    }
                    default:
                        return;
                }

                decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
                decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
                decInstr.shamt = getPartialBitsShifted<24, 20>(encInstr);
                return;
            }

            const uint32_t funct7 = getPartialBitsShifted<31, 25>(encInstr);
            if (funct3 == 0b101 && funct7 == 0b0100000) {  // SRAIW
                decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
                decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
                decInstr.shamt = getPartialBitsShifted<24, 20>(encInstr);
                decInstr.exec = ExecutorSRAIW;
            }
            return;
        }
        case 0b0111011: {  // ADDW or SUBW or SLLW or SRLW or SRAW
            const uint32_t funct3 = getPartialBitsShifted<14, 12>(encInstr);
            const uint32_t funct7 = getPartialBitsShifted<31, 25>(encInstr);
            if (funct3 == 0b000) {
                switch (funct7) {
                    case 0b0000000: {  // ADDW
                        decInstr.type = InstructionType::ADDW;
                        decInstr.exec = ExecutorADDW;
                        break;
                    }
                    case 0b0100000: {  // SUBW
                        decInstr.type = InstructionType::SUBW;
                        decInstr.exec = ExecutorSUBW;
                        break;
                    }
                    default:
                        return;
                }
            } else if (funct7 == 0b0000000) {
                switch (funct3) {
                    case 0b001: {  // SLLW
                        decInstr.type = InstructionType::SLLW;
                        decInstr.exec = ExecutorSLLW;
                        break;
                    }
                    case 0b101: {  // SRLW
                        decInstr.type = InstructionType::SRLW;
                        decInstr.exec = ExecutorSRLW;
                        break;
                    }
                    default:
                        return;
                }
            } else if (funct3 == 0b101 && funct7 == 0b0100000) {  // SRAW
                decInstr.type = InstructionType::SRAW;
                decInstr.exec = ExecutorSRAW;
            } else {
                return;
            }

            decInstr.rd = static_cast<RegisterType>(getPartialBitsShifted<11, 7>(encInstr));
            decInstr.rs1 = static_cast<RegisterType>(getPartialBitsShifted<19, 15>(encInstr));
            decInstr.rs2 = static_cast<RegisterType>(getPartialBitsShifted<24, 20>(encInstr));
            return;
        }
        default:;
    }
}

}  // namespace RISCV
