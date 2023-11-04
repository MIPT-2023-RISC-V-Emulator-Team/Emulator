#ifndef INCLUDE_EXECUTOR_H
#define INCLUDE_EXECUTOR_H

namespace RISCV {

class Hart;
struct DecodedInstruction;

// ================================ PC ================================= //

void ExecutorLUI(Hart* hart, const DecodedInstruction& instr);
void ExecutorAUIPC(Hart* hart, const DecodedInstruction& instr);

// =============================== Jumps =============================== //

void ExecutorJAL(Hart* hart, const DecodedInstruction& instr);
void ExecutorJALR(Hart* hart, const DecodedInstruction& instr);

// ============================= Branching ============================= //

void ExecutorBEQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorBNE(Hart* hart, const DecodedInstruction& instr);
void ExecutorBLT(Hart* hart, const DecodedInstruction& instr);
void ExecutorBGE(Hart* hart, const DecodedInstruction& instr);
void ExecutorBLTU(Hart* hart, const DecodedInstruction& instr);
void ExecutorBGEU(Hart* hart, const DecodedInstruction& instr);

// =============================== Load ================================ //

void ExecutorLB(Hart* hart, const DecodedInstruction& instr);
void ExecutorLH(Hart* hart, const DecodedInstruction& instr);
void ExecutorLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorLD(Hart* hart, const DecodedInstruction& instr);
void ExecutorLBU(Hart* hart, const DecodedInstruction& instr);
void ExecutorLHU(Hart* hart, const DecodedInstruction& instr);
void ExecutorLWU(Hart* hart, const DecodedInstruction& instr);

// =============================== Store =============================== //

void ExecutorSB(Hart* hart, const DecodedInstruction& instr);
void ExecutorSH(Hart* hart, const DecodedInstruction& instr);
void ExecutorSW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSD(Hart* hart, const DecodedInstruction& instr);

// ======================= Arithmetic immediate ======================== //

void ExecutorADDI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLLI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLTI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLTIU(Hart* hart, const DecodedInstruction& instr);
void ExecutorXORI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRLI(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRAI(Hart* hart, const DecodedInstruction& instr);
void ExecutorORI(Hart* hart, const DecodedInstruction& instr);
void ExecutorANDI(Hart* hart, const DecodedInstruction& instr);
void ExecutorADDIW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLLIW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRLIW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRAIW(Hart* hart, const DecodedInstruction& instr);

// ============================ Arithmetic ============================= //

void ExecutorADD(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLL(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLT(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLTU(Hart* hart, const DecodedInstruction& instr);
void ExecutorXOR(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRL(Hart* hart, const DecodedInstruction& instr);
void ExecutorOR(Hart* hart, const DecodedInstruction& instr);
void ExecutorAND(Hart* hart, const DecodedInstruction& instr);
void ExecutorSUB(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRA(Hart* hart, const DecodedInstruction& instr);
void ExecutorADDW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSUBW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSLLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRAW(Hart* hart, const DecodedInstruction& instr);

// ========================== Miscellaneous ============================ //

void ExecutorFENCE(Hart* hart, const DecodedInstruction& instr);
void ExecutorECALL(Hart* hart, const DecodedInstruction& instr);
void ExecutorEBREAK(Hart* hart, const DecodedInstruction& instr);
void ExecutorFENCEI(Hart* hart, const DecodedInstruction& instr);
void ExecutorMUL(Hart* hart, const DecodedInstruction& instr);
void ExecutorMULH(Hart* hart, const DecodedInstruction& instr);
void ExecutorMULHSU(Hart* hart, const DecodedInstruction& instr);
void ExecutorMULHU(Hart* hart, const DecodedInstruction& instr);
void ExecutorDIV(Hart* hart, const DecodedInstruction& instr);
void ExecutorDIVU(Hart* hart, const DecodedInstruction& instr);
void ExecutorREM(Hart* hart, const DecodedInstruction& instr);
void ExecutorREMU(Hart* hart, const DecodedInstruction& instr);
void ExecutorMULW(Hart* hart, const DecodedInstruction& instr);
void ExecutorDIVW(Hart* hart, const DecodedInstruction& instr);
void ExecutorDIVUW(Hart* hart, const DecodedInstruction& instr);
void ExecutorREMW(Hart* hart, const DecodedInstruction& instr);
void ExecutorREMUW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOADDW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOXORW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOORW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOANDW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMINW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMAXW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMINUW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMAXUW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOSWAPW(Hart* hart, const DecodedInstruction& instr);
void ExecutorLRW(Hart* hart, const DecodedInstruction& instr);
void ExecutorSCW(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOADDD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOXORD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOORD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOANDD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMIND(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMAXD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMINUD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOMAXUD(Hart* hart, const DecodedInstruction& instr);
void ExecutorAMOSWAPD(Hart* hart, const DecodedInstruction& instr);
void ExecutorLRD(Hart* hart, const DecodedInstruction& instr);
void ExecutorSCD(Hart* hart, const DecodedInstruction& instr);
void ExecutorURET(Hart* hart, const DecodedInstruction& instr);
void ExecutorSRET(Hart* hart, const DecodedInstruction& instr);
void ExecutorMRET(Hart* hart, const DecodedInstruction& instr);
void ExecutorDRET(Hart* hart, const DecodedInstruction& instr);
void ExecutorSFENCEVMA(Hart* hart, const DecodedInstruction& instr);
void ExecutorWFI(Hart* hart, const DecodedInstruction& instr);
void ExecutorCSRRW(Hart* hart, const DecodedInstruction& instr);
void ExecutorCSRRS(Hart* hart, const DecodedInstruction& instr);
void ExecutorCSRRC(Hart* hart, const DecodedInstruction& instr);
void ExecutorCSRRWI(Hart* hart, const DecodedInstruction& instr);
void ExecutorCSRRSI(Hart* hart, const DecodedInstruction& instr);
void ExecutorCSRRCI(Hart* hart, const DecodedInstruction& instr);
void ExecutorHFENCEVVMA(Hart* hart, const DecodedInstruction& instr);
void ExecutorHFENCEGVMA(Hart* hart, const DecodedInstruction& instr);
void ExecutorFADDS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSUBS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMULS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFDIVS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJNS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJXS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMINS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMAXS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSQRTS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFADDD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSUBD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMULD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFDIVD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJND(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJXD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMIND(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMAXD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTSD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTDS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSQRTD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFADDQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSUBQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMULQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFDIVQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJNQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSGNJXQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMINQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMAXQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTSQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTQS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTDQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTQD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSQRTQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLES(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLTS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFEQS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLED(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLTD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFEQD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLEQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLTQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFEQQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTWS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTWUS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTLS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTLUS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMVXW(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCLASSS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTWD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTWUD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTLD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTLUD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMVXD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCLASSD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTWQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTWUQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTLQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTLUQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMVXQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCLASSQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTSW(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTSWU(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTSL(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTSLU(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMVWX(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTDW(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTDWU(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTDL(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTDLU(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMVDX(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTQW(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTQWU(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTQL(Hart* hart, const DecodedInstruction& instr);
void ExecutorFCVTQLU(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMVQX(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLW(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFLQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSW(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFSQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMADDS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMSUBS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFNMSUBS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFNMADDS(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMADDD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMSUBD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFNMSUBD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFNMADDD(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMADDQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFMSUBQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFNMSUBQ(Hart* hart, const DecodedInstruction& instr);
void ExecutorFNMADDQ(Hart* hart, const DecodedInstruction& instr);

}  // namespace RISCV

#endif  // INCLUDE_EXECUTOR_H
