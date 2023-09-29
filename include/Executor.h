#ifndef INCLUDE_EXECUTOR_H
#define INCLUDE_EXECUTOR_H

namespace RISCV {

class Hart;
struct DecodedInstruction;

// ========================= Generic IExecutor ========================= //

class IExecutor {
 public:
  virtual void operator()(Hart* hart, const DecodedInstruction& instr) = 0;
};

// Implement singleton
#define MAKE_EXECUTOR_DECL(TYPE)                                           \
  class Executor##TYPE final : public IExecutor {                          \
   private:                                                                \
    static Executor##TYPE* instancePtr;                                    \
    Executor##TYPE(){};                                                    \
                                                                           \
   public:                                                                 \
    Executor##TYPE(const Executor##TYPE& obj) = delete;                    \
                                                                           \
    static Executor##TYPE* getInstance() {                                 \
      if (!instancePtr) {                                                  \
        instancePtr = new Executor##TYPE();                                \
      }                                                                    \
                                                                           \
      return instancePtr;                                                  \
    }                                                                      \
                                                                           \
    void operator()(Hart* hart, const DecodedInstruction& instr) override; \
  };

// Used in .cpp file
#define MAKE_EXECUTOR_INIT(TYPE) Executor##TYPE* Executor##TYPE::instancePtr = nullptr;

// ================================ PC ================================= //

MAKE_EXECUTOR_DECL(LUI)
MAKE_EXECUTOR_DECL(AUIPC)

// =============================== Jumps =============================== //

MAKE_EXECUTOR_DECL(JAL)
MAKE_EXECUTOR_DECL(JALR)

// ============================= Branching ============================= //

MAKE_EXECUTOR_DECL(BEQ)
MAKE_EXECUTOR_DECL(BNE)
MAKE_EXECUTOR_DECL(BLT)
MAKE_EXECUTOR_DECL(BGE)
MAKE_EXECUTOR_DECL(BLTU)
MAKE_EXECUTOR_DECL(BGEU)

// =============================== Load ================================ //

MAKE_EXECUTOR_DECL(LB)
MAKE_EXECUTOR_DECL(LH)
MAKE_EXECUTOR_DECL(LW)
MAKE_EXECUTOR_DECL(LD)
MAKE_EXECUTOR_DECL(LBU)
MAKE_EXECUTOR_DECL(LHU)
MAKE_EXECUTOR_DECL(LWU)

// =============================== Store =============================== //

MAKE_EXECUTOR_DECL(SB)
MAKE_EXECUTOR_DECL(SH)
MAKE_EXECUTOR_DECL(SW)
MAKE_EXECUTOR_DECL(SD)

// ======================= Arithmetic immediate ======================== //

MAKE_EXECUTOR_DECL(ADDI)
MAKE_EXECUTOR_DECL(SLLI)
MAKE_EXECUTOR_DECL(SLTI)
MAKE_EXECUTOR_DECL(SLTIU)
MAKE_EXECUTOR_DECL(XORI)
MAKE_EXECUTOR_DECL(SRLI)
MAKE_EXECUTOR_DECL(SRAI)
MAKE_EXECUTOR_DECL(ORI)
MAKE_EXECUTOR_DECL(ANDI)
MAKE_EXECUTOR_DECL(ADDIW)
MAKE_EXECUTOR_DECL(SLLIW)
MAKE_EXECUTOR_DECL(SRLIW)
MAKE_EXECUTOR_DECL(SRAIW)

// ============================ Arithmetic ============================= //

MAKE_EXECUTOR_DECL(ADD)
MAKE_EXECUTOR_DECL(SLL)
MAKE_EXECUTOR_DECL(SLT)
MAKE_EXECUTOR_DECL(SLTU)
MAKE_EXECUTOR_DECL(XOR)
MAKE_EXECUTOR_DECL(SRL)
MAKE_EXECUTOR_DECL(OR)
MAKE_EXECUTOR_DECL(AND)
MAKE_EXECUTOR_DECL(SUB)
MAKE_EXECUTOR_DECL(SRA)
MAKE_EXECUTOR_DECL(ADDW)
MAKE_EXECUTOR_DECL(SUBW)
MAKE_EXECUTOR_DECL(SLLW)
MAKE_EXECUTOR_DECL(SRLW)
MAKE_EXECUTOR_DECL(SRAW)

// ========================== Miscellaneous ============================ //

MAKE_EXECUTOR_DECL(FENCE)
MAKE_EXECUTOR_DECL(ECALL)
MAKE_EXECUTOR_DECL(EBREAK)

}  // namespace RISCV

#endif  // INCLUDE_EXECUTOR_H
