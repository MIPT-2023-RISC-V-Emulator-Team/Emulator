#include "CpuRV.h"

#include <iostream>

namespace RISCV {

static inline uint32_t getPartialBits(const uint32_t val,
                                      const uint32_t first,
                                      const uint32_t second) {
  const uint32_t mask = ((1 << (first - second + 1)) - 1) << second;
  return (val & mask);
}

// signBitNum = 0, 1, 2, ..., 31 from right to left
static inline uint64_t signExtend(const uint64_t val, const uint8_t signBitNum) {
  return ~(((val & (1 << signBitNum)) - 1)) | val;
}

void CpuRV::fetch(EncodedInstruction& encInstr) {
  if (!mmu_.load32(pc_, &encInstr.instr)) {
    printf("Could not handle page fault\n");
    exit(EXIT_FAILURE);
  }
}

void CpuRV::decode(const EncodedInstruction& encInstr, DecodedInstruction& decInstr) const {
  const uint32_t opcode = encInstr.instr & 0b1111111;

  switch (opcode) {
    case 0: {  // INVALID
      goto got_invalid_instruction;
    }
    case 0b0110111: {  // LUI
      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.imm = getPartialBits(encInstr.instr, 31, 12);
      decInstr.immSignBitNum = 31;

      decInstr.type = InstructionType::LUI;
      return;
    }
    case 0b0010111: {  // AUIPC
      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.imm = getPartialBits(encInstr.instr, 31, 12);
      decInstr.immSignBitNum = 31;

      decInstr.type = InstructionType::AUIPC;
      return;
    }
    case 0b1101111: {  // JAL
      decInstr.type = InstructionType::JAL;
      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);

      decInstr.imm = (getPartialBits(encInstr.instr, 30, 21) >> 20) +
                     (getPartialBits(encInstr.instr, 20, 20) >> 9) +
                     (getPartialBits(encInstr.instr, 19, 12)) +
                     (getPartialBits(encInstr.instr, 31, 31) >> 11);

      decInstr.immSignBitNum = 20;
      return;
    }
    case 0b1100111: {  // JALR
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      if (funct3 == 0b000) {
        decInstr.type = InstructionType::JALR;
        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
        decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
        decInstr.immSignBitNum = 11;
        return;
      } else {
        goto got_invalid_instruction;
      }
    }
    case 0b1100011: {  // BEQ or BNE or BLT or BGE or BLTU or BGEU
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      switch (funct3) {
        case 0b000: {  // BEQ
          decInstr.type = InstructionType::BEQ;
          break;
        }
        case 0b001: {  // BNE
          decInstr.type = InstructionType::BNE;
          break;
        }
        case 0b100: {  // BLT
          decInstr.type = InstructionType::BLT;
          break;
        }
        case 0b101: {  // BGE
          decInstr.type = InstructionType::BGE;
          break;
        }
        case 0b110: {  // BLTU
          decInstr.type = InstructionType::BLTU;
          break;
        }
        case 0b111: {  // BGEU
          decInstr.type = InstructionType::BGEU;
          break;
        }
        default:
          goto got_invalid_instruction;
      }

      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 24, 20) >> 20);

      decInstr.imm = (getPartialBits(encInstr.instr, 11, 8) >> 7) +
                     (getPartialBits(encInstr.instr, 30, 25) >> 20) +
                     (getPartialBits(encInstr.instr, 7, 7) << 4) +
                     (getPartialBits(encInstr.instr, 31, 31) >> 19);

      decInstr.immSignBitNum = 12;
      return;
    }
    case 0b0000011: {  // LB or LH or LW or LD or LBU or LHU or LWU
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      switch (funct3) {
        case 0b000: {  // LB
          decInstr.type = InstructionType::LB;
          break;
        }
        case 0b001: {  // LH
          decInstr.type = InstructionType::LH;
          break;
        }
        case 0b010: {  // LW
          decInstr.type = InstructionType::LW;
          break;
        }
        case 0b011: {  // LD
          decInstr.type = InstructionType::LD;
          break;
        }
        case 0b100: {  // LBU
          decInstr.type = InstructionType::LBU;
          break;
        }
        case 0b101: {  // LHU
          decInstr.type = InstructionType::LHU;
          break;
        }
        case 0b110: {  // LWU
          decInstr.type = InstructionType::LWU;
          break;
        }
        default:
          goto got_invalid_instruction;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
      decInstr.immSignBitNum = 11;
      return;
    }
    case 0b0100011: {  // SB or SH or SW or SD
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      switch (funct3) {
        case 0b000: {  // SB
          decInstr.type = InstructionType::SB;
          break;
        }
        case 0b001: {  // SH
          decInstr.type = InstructionType::SH;
          break;
        }
        case 0b010: {  // SW
          decInstr.type = InstructionType::SW;
          break;
        }
        case 0b011: {  // SD
          decInstr.type = InstructionType::SD;
          break;
        }
        default:
          goto got_invalid_instruction;
      }

      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 24, 20) >> 20);
      decInstr.imm = (getPartialBits(encInstr.instr, 11, 7) >> 7) +
                     (getPartialBits(encInstr.instr, 31, 25) >> 20);
      decInstr.immSignBitNum = 11;
      return;
    }
    case 0b0010011: {  // ADDI or SLLI or SLTI or SLTIU or XORI or SRLI or SRAI or ORI or ANDI
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      const uint32_t funct7 = getPartialBits(encInstr.instr, 31, 25) >> 25;
      switch (funct3) {
        case 0b000: {  // ADDI
          decInstr.type = InstructionType::ADDI;
          decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          break;
        }
        case 0b001: {  // SLLI
          if (funct7 == 0b0000000) {
            decInstr.type = InstructionType::SLLI;
            decInstr.shamt = getPartialBits(encInstr.instr, 25, 20) >> 20;
            break;
          } else {
            goto got_invalid_instruction;
          }
        }
        case 0b010: {  // SLTI
          decInstr.type = InstructionType::SLTI;
          decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          break;
        }
        case 0b011: {  // SLTIU
          decInstr.type = InstructionType::SLTIU;
          decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          break;
        }
        case 0b100: {  // XORI
          decInstr.type = InstructionType::XORI;
          decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          break;
        }
        case 0b101: {                 // SRLI or SRAI
          if (funct7 == 0b0000000) {  // SRLI
            decInstr.type = InstructionType::SRLI;
            decInstr.shamt = getPartialBits(encInstr.instr, 25, 20) >> 20;
            break;
          } else if (funct7 == 0b0100000) {  // SRAI
            decInstr.type = InstructionType::SRAI;
            decInstr.shamt = getPartialBits(encInstr.instr, 25, 20) >> 20;
            break;
          } else {
            goto got_invalid_instruction;
          }
        }
        case 0b110: {  // ORI
          decInstr.type = InstructionType::ORI;
          decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          break;
        }
        case 0b111: {  // ANDI
          decInstr.type = InstructionType::ANDI;
          decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
          decInstr.immSignBitNum = 11;
          break;
        }
        default:
          goto got_invalid_instruction;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      return;
    }
    case 0b0110011: {  // ADD or SLL or SLT or SLTU or XOR or SRL or OR or AND or SUB or SRA
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      const uint32_t funct7 = getPartialBits(encInstr.instr, 31, 25) >> 25;
      if (funct7 == 0b0000000) {
        switch (funct3) {
          case 0b000: {  // ADD
            decInstr.type = InstructionType::ADD;
            break;
          }
          case 0b001: {  // SLL
            decInstr.type = InstructionType::SLL;
            break;
          }
          case 0b010: {  // SLT
            decInstr.type = InstructionType::SLT;
            break;
          }
          case 0b011: {  // SLTU
            decInstr.type = InstructionType::SLTU;
            break;
          }
          case 0b100: {  // XOR
            decInstr.type = InstructionType::XOR;
            break;
          }
          case 0b101: {  // SRL
            decInstr.type = InstructionType::SRL;
            break;
          }
          case 0b110: {  // OR
            decInstr.type = InstructionType::OR;
            break;
          }
          case 0b111: {  // AND
            decInstr.type = InstructionType::AND;
            break;
          }
          default:
            goto got_invalid_instruction;
        }
      } else if (funct7 == 0b0100000) {
        switch (funct3) {
          case 0b000: {  // SUB
            decInstr.type = InstructionType::SUB;
            break;
          }
          case 0b101: {  // SRA
            decInstr.type = InstructionType::SRA;
            break;
          }
          default:
            goto got_invalid_instruction;
        }
      } else {
        goto got_invalid_instruction;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 24, 20) >> 20);
      return;
    }
    case 0b0001111: {  // FENCE
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      if (funct3 == 0b000) {
        decInstr.type = InstructionType::FENCE;

        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
        decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
        decInstr.immSignBitNum = 11;
        return;
      } else {
        goto got_invalid_instruction;
      }
    }
    case 0b1110011: {  // ECALL or EBREAK
      const uint32_t funct12 = getPartialBits(encInstr.instr, 31, 20) >> 20;
      if (funct12 == 0b000000000000) {  // ECALL
        decInstr.type = InstructionType::ECALL;
      } else if (funct12 == 0b000000000001) {  // EBREAK
        decInstr.type = InstructionType::EBREAK;
      } else {
        goto got_invalid_instruction;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      return;
    }
    case 0b0011011: {  // ADDIW or SLLIW or SRLIW or SRAIW
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      if (funct3 == 0b000) {  // ADDIW
        decInstr.type = InstructionType::ADDIW;
        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
        decInstr.imm = getPartialBits(encInstr.instr, 31, 20) >> 20;
        decInstr.immSignBitNum = 11;
        return;
      }

      const uint32_t funct6 = getPartialBits(encInstr.instr, 31, 26) >> 26;
      if (funct6 == 0b000000) {
        switch (funct3) {
          case 0b001: {  // SLLIW
            decInstr.type = InstructionType::SLLIW;
            break;
          }
          case 0b101: {  // SRLIW
            decInstr.type = InstructionType::SRLIW;
            break;
          }
          default:
            goto got_invalid_instruction;

            decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
            decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
            decInstr.shamt = getPartialBits(encInstr.instr, 24, 20) >> 20;
            return;
        }
      }

      const uint32_t funct7 = getPartialBits(encInstr.instr, 31, 25) >> 25;
      if (funct3 == 0b101 && funct7 == 0b0100000) {  // SRAIW
        decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
        decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
        decInstr.shamt = getPartialBits(encInstr.instr, 24, 20) >> 20;
        return;
      }

      goto got_invalid_instruction;
    }
    case 0b0111011: {  // ADDW or SUBW or SLLW or SRLW or SRAW
      const uint32_t funct3 = getPartialBits(encInstr.instr, 14, 12) >> 12;
      const uint32_t funct7 = getPartialBits(encInstr.instr, 31, 25) >> 25;
      if (funct3 == 0b000) {
        switch (funct7) {
          case 0b0000000: {  // ADDW
            decInstr.type = InstructionType::ADDW;
            break;
          }
          case 0b0100000: {  // SUBW
            decInstr.type = InstructionType::SUBW;
            break;
          }
          default:
            goto got_invalid_instruction;
        }
      } else if (funct7 == 0b0000000) {
        switch (funct3) {
          case 0b001: {  // SLLW
            decInstr.type = InstructionType::SLLW;
            break;
          }
          case 0b101: {  // SRLW
            decInstr.type = InstructionType::SRLW;
            break;
          }
          default:
            goto got_invalid_instruction;
        }
      } else if (funct3 == 0b101 && funct7 == 0b0100000) {  // SRAW
        decInstr.type = InstructionType::SRAW;
      } else {
        goto got_invalid_instruction;
      }

      decInstr.rd = static_cast<RegisterType>(getPartialBits(encInstr.instr, 11, 7) >> 7);
      decInstr.rs1 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 19, 15) >> 15);
      decInstr.rs2 = static_cast<RegisterType>(getPartialBits(encInstr.instr, 24, 20) >> 20);
      return;
    }
    default:
      goto got_invalid_instruction;
  }

got_invalid_instruction:
  decInstr.type = InstructionType::INSTRUCTION_INVALID;
  return;
}

void CpuRV::execute(const DecodedInstruction& decInstr) {
  std::cout << std::hex << pc_ << ": \t" << InstructionNames[decInstr.type] << std::endl;

  uint64_t nextPc = pc_ + INSTRUCTION_BYTESIZE;

  switch (decInstr.type) {
    case InstructionType::LUI: {
      regs_[decInstr.rd].value = signExtend(decInstr.imm, decInstr.immSignBitNum);
      break;
    }
    case InstructionType::AUIPC: {
      regs_[decInstr.rd].value = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      break;
    }
    case InstructionType::JAL: {
      regs_[decInstr.rd].value = pc_ + INSTRUCTION_BYTESIZE;
      nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      break;
    }
    case InstructionType::JALR: {
      const uint64_t tmp = pc_ + INSTRUCTION_BYTESIZE;
      nextPc =
          (regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum)) & (~1);
      regs_[decInstr.rd].value = tmp;
      break;
    }
    case InstructionType::LB: {
      uint8_t tmp;
      mmu_.load8(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                 &tmp);
      regs_[decInstr.rd].value = signExtend(tmp, 7);
      break;
    }
    case InstructionType::LH: {
      uint16_t tmp;
      mmu_.load16(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                  &tmp);
      regs_[decInstr.rd].value = signExtend(tmp, 15);
      break;
    }
    case InstructionType::LW: {
      uint32_t tmp;
      mmu_.load32(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                  &tmp);
      regs_[decInstr.rd].value = signExtend(tmp, 31);
      break;
    }
    case InstructionType::LD: {
      mmu_.load64(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                  &regs_[decInstr.rd].value);
      break;
    }
    case InstructionType::LBU: {
      uint8_t tmp;
      mmu_.load8(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                 &tmp);
      regs_[decInstr.rd].value = tmp;
      break;
    }
    case InstructionType::LHU: {
      uint16_t tmp;
      mmu_.load16(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                  &tmp);
      regs_[decInstr.rd].value = tmp;
      break;
    }
    case InstructionType::LWU: {
      uint32_t tmp;
      mmu_.load32(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                  &tmp);
      regs_[decInstr.rd].value = tmp;
      break;
    }
    case InstructionType::SB: {
      mmu_.store8(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                  regs_[decInstr.rs2].value & 0xFF);
      break;
    }
    case InstructionType::SH: {
      mmu_.store16(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                   regs_[decInstr.rs2].value & 0xFFFF);
      break;
    }
    case InstructionType::SW: {
      mmu_.store32(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                   regs_[decInstr.rs2].value & 0xFFFFFFFF);
      break;
    }
    case InstructionType::SD: {
      mmu_.store64(regs_[decInstr.rs1].value + signExtend(decInstr.imm, decInstr.immSignBitNum),
                   regs_[decInstr.rs2].value);
      break;
    }
    case InstructionType::BEQ: {
      if (regs_[decInstr.rs1].value == regs_[decInstr.rs2].value) {
        nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      }
      break;
    }
    case InstructionType::BNE: {
      if (regs_[decInstr.rs1].value != regs_[decInstr.rs2].value) {
        nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      }
      break;
    }
    case InstructionType::BLT: {
      if (static_cast<int64_t>(regs_[decInstr.rs1].value) < regs_[decInstr.rs2].value) {
        nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      }
      break;
    }
    case InstructionType::BGE: {
      if (static_cast<int64_t>(regs_[decInstr.rs1].value) >= regs_[decInstr.rs2].value) {
        nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      }
      break;
    }
    case InstructionType::BLTU: {
      if (regs_[decInstr.rs1].value < regs_[decInstr.rs2].value) {
        nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      }
      break;
    }
    case InstructionType::BGEU: {
      if (regs_[decInstr.rs1].value >= regs_[decInstr.rs2].value) {
        nextPc = pc_ + signExtend(decInstr.imm, decInstr.immSignBitNum);
      }
      break;
    }
  }

  pc_ = nextPc;
}

}  // namespace RISCV
