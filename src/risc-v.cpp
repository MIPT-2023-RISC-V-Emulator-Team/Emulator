#include <stdlib.h> 

#include "../include/CpuRV.h"


int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stdout, "Usage: %s <elf_filename>\n", argv[0]);
        return -1;
    }

    RISCV::CpuRV CPU;
    CPU.loadElfFile(argv[1]);

    RISCV::EncodedInstruction encInstr;
    RISCV::DecodedInstruction decInstr;

    while(true) {
        CPU.fetch(encInstr);
        CPU.decode(encInstr, decInstr);

        if(decInstr.type == RISCV::InstructionType::INSTRUCTION_INVALID)
            break;

        CPU.execute(decInstr);
    }

    return 0;
}
