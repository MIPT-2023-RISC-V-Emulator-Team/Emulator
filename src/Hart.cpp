#include "Hart.h"

#include <iostream>

#include "Executor.h"

namespace RISCV {

static inline uint32_t getPartialBits(const uint32_t val,
                                      const uint32_t first,
                                      const uint32_t second) {
  const uint32_t mask = ((1 << (first - second + 1)) - 1) << second;
  return (val & mask);
}

void Hart::fetch(EncodedInstruction& encInstr) {
  if (!mmu_.load32(pc_, &encInstr)) {
    printf("Could not handle page fault\n");
    exit(EXIT_FAILURE);
  }
}

void Hart::decode(const EncodedInstruction encInstr, DecodedInstruction& decInstr) const {
  // Initialize instruction to be invalid so every return will be either valid instruction or
  // invalid one
  decInstr.type = InstructionType::INSTRUCTION_INVALID;

  const uint32_t opcode = encInstr & 0b1111111;

  switch (opcode) {
    case 0b0110111: {  // LUI
      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.imm = getPartialBits(encInstr, 31, 12);
      decInstr.immSignBitNum = 31;
      decInstr.exec = ExecutorLUI::getInstance();

      decInstr.type = InstructionType::LUI;
      return;
    }
    case 0b0010111: {  // AUIPC
      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.imm = getPartialBits(encInstr, 31, 12);
      decInstr.immSignBitNum = 31;
      decInstr.exec = ExecutorAUIPC::getInstance();

      decInstr.type = InstructionType::AUIPC;
      return;
    }
    case 0b1101111: {  // JAL
      decInstr.type = InstructionType::JAL;
      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);

      decInstr.imm = (getPartialBits(encInstr, 30, 21) >> 20) +
                     (getPartialBits(encInstr, 20, 20) >> 9) + (getPartialBits(encInstr, 19, 12)) +
                     (getPartialBits(encInstr, 31, 31) >> 11);

      decInstr.immSignBitNum = 20;
      decInstr.exec = ExecutorJAL::getInstance();
      return;
    }
    case 0b1100111: {  // JALR
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      if (funct3 == 0b000) {
        decInstr.type = InstructionType::JALR;
        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
        decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
        decInstr.immSignBitNum = 11;
        decInstr.exec = ExecutorJALR::getInstance();
      }
      return;
    }
    case 0b1100011: {  // BEQ or BNE or BLT or BGE or BLTU or BGEU
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      switch (funct3) {
        case 0b000: {  // BEQ
          decInstr.type = InstructionType::BEQ;
          decInstr.exec = ExecutorBEQ::getInstance();
          break;
        }
        case 0b001: {  // BNE
          decInstr.type = InstructionType::BNE;
          decInstr.exec = ExecutorBNE::getInstance();
          break;
        }
        case 0b100: {  // BLT
          decInstr.type = InstructionType::BLT;
          decInstr.exec = ExecutorBLT::getInstance();
          break;
        }
        case 0b101: {  // BGE
          decInstr.type = InstructionType::BGE;
          decInstr.exec = ExecutorBGE::getInstance();
          break;
        }
        case 0b110: {  // BLTU
          decInstr.type = InstructionType::BLTU;
          decInstr.exec = ExecutorBLTU::getInstance();
          break;
        }
        case 0b111: {  // BGEU
          decInstr.type = InstructionType::BGEU;
          decInstr.exec = ExecutorBGEU::getInstance();
          break;
        }
        default:
          return;
      }

      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr, 24, 20) >> 20);

      decInstr.imm =
          (getPartialBits(encInstr, 11, 8) >> 7) + (getPartialBits(encInstr, 30, 25) >> 20) +
          (getPartialBits(encInstr, 7, 7) << 4) + (getPartialBits(encInstr, 31, 31) >> 19);

      decInstr.immSignBitNum = 12;
      return;
    }
    case 0b0000011: {  // LB or LH or LW or LD or LBU or LHU or LWU
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      switch (funct3) {
        case 0b000: {  // LB
          decInstr.type = InstructionType::LB;
          decInstr.exec = ExecutorLB::getInstance();
          break;
        }
        case 0b001: {  // LH
          decInstr.type = InstructionType::LH;
          decInstr.exec = ExecutorLH::getInstance();
          break;
        }
        case 0b010: {  // LW
          decInstr.type = InstructionType::LW;
          decInstr.exec = ExecutorLW::getInstance();
          break;
        }
        case 0b011: {  // LD
          decInstr.type = InstructionType::LD;
          decInstr.exec = ExecutorLD::getInstance();
          break;
        }
        case 0b100: {  // LBU
          decInstr.type = InstructionType::LBU;
          decInstr.exec = ExecutorLBU::getInstance();
          break;
        }
        case 0b101: {  // LHU
          decInstr.type = InstructionType::LHU;
          decInstr.exec = ExecutorLHU::getInstance();
          break;
        }
        case 0b110: {  // LWU
          decInstr.type = InstructionType::LWU;
          decInstr.exec = ExecutorLWU::getInstance();
          break;
        }
        default:
          return;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
      decInstr.immSignBitNum = 11;
      return;
    }
    case 0b0100011: {  // SB or SH or SW or SD
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      switch (funct3) {
        case 0b000: {  // SB
          decInstr.type = InstructionType::SB;
          decInstr.exec = ExecutorSB::getInstance();
          break;
        }
        case 0b001: {  // SH
          decInstr.type = InstructionType::SH;
          decInstr.exec = ExecutorSH::getInstance();
          break;
        }
        case 0b010: {  // SW
          decInstr.type = InstructionType::SW;
          decInstr.exec = ExecutorSW::getInstance();
          break;
        }
        case 0b011: {  // SD
          decInstr.type = InstructionType::SD;
          decInstr.exec = ExecutorSD::getInstance();
          break;
        }
        default:
          return;
      }

      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr, 24, 20) >> 20);
      decInstr.imm =
          (getPartialBits(encInstr, 11, 7) >> 7) + (getPartialBits(encInstr, 31, 25) >> 20);
      decInstr.immSignBitNum = 11;
      return;
    }
    case 0b0010011: {  // ADDI or SLLI or SLTI or SLTIU or XORI or SRLI or SRAI or ORI or ANDI
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      const uint32_t funct7 = getPartialBits(encInstr, 31, 25) >> 25;
      switch (funct3) {
        case 0b000: {  // ADDI
          decInstr.type = InstructionType::ADDI;
          decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          decInstr.exec = ExecutorADDI::getInstance();
          break;
        }
        case 0b001: {  // SLLI
          if (funct7 == 0b0000000) {
            decInstr.type = InstructionType::SLLI;
            decInstr.shamt = getPartialBits(encInstr, 25, 20) >> 20;
            decInstr.exec = ExecutorSLLI::getInstance();
            break;
          }
          return;
        }
        case 0b010: {  // SLTI
          decInstr.type = InstructionType::SLTI;
          decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          decInstr.exec = ExecutorSLTI::getInstance();
          break;
        }
        case 0b011: {  // SLTIU
          decInstr.type = InstructionType::SLTIU;
          decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          decInstr.exec = ExecutorSLTIU::getInstance();
          break;
        }
        case 0b100: {  // XORI
          decInstr.type = InstructionType::XORI;
          decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          decInstr.exec = ExecutorXORI::getInstance();
          break;
        }
        case 0b101: {                 // SRLI or SRAI
          if (funct7 == 0b0000000) {  // SRLI
            decInstr.type = InstructionType::SRLI;
            decInstr.shamt = getPartialBits(encInstr, 25, 20) >> 20;
            decInstr.exec = ExecutorSRLI::getInstance();
            break;
          } else if (funct7 == 0b0100000) {  // SRAI
            decInstr.type = InstructionType::SRAI;
            decInstr.shamt = getPartialBits(encInstr, 25, 20) >> 20;
            decInstr.exec = ExecutorSRAI::getInstance();
            break;
          }
          return;
        }
        case 0b110: {  // ORI
          decInstr.type = InstructionType::ORI;
          decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          decInstr.exec = ExecutorORI::getInstance();
          break;
        }
        case 0b111: {  // ANDI
          decInstr.type = InstructionType::ANDI;
          decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          decInstr.exec = ExecutorANDI::getInstance();
          break;
        }
        default:
          return;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      return;
    }
    case 0b0110011: {  // ADD or SLL or SLT or SLTU or XOR or SRL or OR or AND or SUB or SRA
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      const uint32_t funct7 = getPartialBits(encInstr, 31, 25) >> 25;
      if (funct7 == 0b0000000) {
        switch (funct3) {
          case 0b000: {  // ADD
            decInstr.type = InstructionType::ADD;
            decInstr.exec = ExecutorADD::getInstance();
            break;
          }
          case 0b001: {  // SLL
            decInstr.type = InstructionType::SLL;
            decInstr.exec = ExecutorSLL::getInstance();
            break;
          }
          case 0b010: {  // SLT
            decInstr.type = InstructionType::SLT;
            decInstr.exec = ExecutorSLT::getInstance();
            break;
          }
          case 0b011: {  // SLTU
            decInstr.type = InstructionType::SLTU;
            decInstr.exec = ExecutorSLTU::getInstance();
            break;
          }
          case 0b100: {  // XOR
            decInstr.type = InstructionType::XOR;
            decInstr.exec = ExecutorXOR::getInstance();
            break;
          }
          case 0b101: {  // SRL
            decInstr.type = InstructionType::SRL;
            decInstr.exec = ExecutorSRL::getInstance();
            break;
          }
          case 0b110: {  // OR
            decInstr.type = InstructionType::OR;
            decInstr.exec = ExecutorOR::getInstance();
            break;
          }
          case 0b111: {  // AND
            decInstr.type = InstructionType::AND;
            decInstr.exec = ExecutorAND::getInstance();
            break;
          }
          default:
            return;
        }
      } else if (funct7 == 0b0100000) {
        switch (funct3) {
          case 0b000: {  // SUB
            decInstr.type = InstructionType::SUB;
            decInstr.exec = ExecutorSUB::getInstance();
            break;
          }
          case 0b101: {  // SRA
            decInstr.type = InstructionType::SRA;
            decInstr.exec = ExecutorSRA::getInstance();
            break;
          }
          default:
            return;
        }
      } else {
        return;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr, 24, 20) >> 20);
      return;
    }
    case 0b0001111: {  // FENCE
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      if (funct3 == 0b000) {
        decInstr.type = InstructionType::FENCE;

        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
        decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
        decInstr.immSignBitNum = 11;
        decInstr.exec = ExecutorFENCE::getInstance();
      }
      return;
    }
    case 0b1110011: {  // ECALL or EBREAK
      const uint32_t funct12 = getPartialBits(encInstr, 31, 20) >> 20;
      if (funct12 == 0b000000000000) {  // ECALL
        decInstr.type = InstructionType::ECALL;
        decInstr.exec = ExecutorECALL::getInstance();
      } else if (funct12 == 0b000000000001) {  // EBREAK
        decInstr.type = InstructionType::EBREAK;
        decInstr.exec = ExecutorEBREAK::getInstance();
      } else {
        return;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      return;
    }
    case 0b0011011: {  // ADDIW or SLLIW or SRLIW or SRAIW
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      if (funct3 == 0b000) {  // ADDIW
        decInstr.type = InstructionType::ADDIW;
        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
        decInstr.imm = getPartialBits(encInstr, 31, 20) >> 20;
        decInstr.immSignBitNum = 11;
        decInstr.exec = ExecutorADDIW::getInstance();
        return;
      }

      const uint32_t funct6 = getPartialBits(encInstr, 31, 26) >> 26;
      if (funct6 == 0b000000) {
        switch (funct3) {
          case 0b001: {  // SLLIW
            decInstr.type = InstructionType::SLLIW;
            decInstr.exec = ExecutorSLLIW::getInstance();
            break;
          }
          case 0b101: {  // SRLIW
            decInstr.type = InstructionType::SRLIW;
            decInstr.exec = ExecutorSRLIW::getInstance();
            break;
          }
          default:
            return;
        }

        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
        decInstr.shamt = getPartialBits(encInstr, 24, 20) >> 20;
        return;
      }

      const uint32_t funct7 = getPartialBits(encInstr, 31, 25) >> 25;
      if (funct3 == 0b101 && funct7 == 0b0100000) {  // SRAIW
        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
        decInstr.shamt = getPartialBits(encInstr, 24, 20) >> 20;
        decInstr.exec = ExecutorSRAIW::getInstance();
      }
      return;
    }
    case 0b0111011: {  // ADDW or SUBW or SLLW or SRLW or SRAW
      const uint32_t funct3 = getPartialBits(encInstr, 14, 12) >> 12;
      const uint32_t funct7 = getPartialBits(encInstr, 31, 25) >> 25;
      if (funct3 == 0b000) {
        switch (funct7) {
          case 0b0000000: {  // ADDW
            decInstr.type = InstructionType::ADDW;
            decInstr.exec = ExecutorADDW::getInstance();
            break;
          }
          case 0b0100000: {  // SUBW
            decInstr.type = InstructionType::SUBW;
            decInstr.exec = ExecutorSUBW::getInstance();
            break;
          }
          default:
            return;
        }
      } else if (funct7 == 0b0000000) {
        switch (funct3) {
          case 0b001: {  // SLLW
            decInstr.type = InstructionType::SLLW;
            decInstr.exec = ExecutorSLLW::getInstance();
            break;
          }
          case 0b101: {  // SRLW
            decInstr.type = InstructionType::SRLW;
            decInstr.exec = ExecutorSRLW::getInstance();
            break;
          }
          default:
            return;
        }
      } else if (funct3 == 0b101 && funct7 == 0b0100000) {  // SRAW
        decInstr.type = InstructionType::SRAW;
        decInstr.exec = ExecutorSRAW::getInstance();
      } else {
        return;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr, 24, 20) >> 20);
      return;
    }
    default:;
  }

  return;
}

void Hart::execute(const DecodedInstruction& decInstr) {
  (*decInstr.exec)(this, decInstr);
}

}  // namespace RISCV
