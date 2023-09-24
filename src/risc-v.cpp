#include <stdlib.h>

#include "CpuRV.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stdout, "Usage: %s <elf_filename>\n", argv[0]);
    return -1;
  }

  RISCV::CpuRV CPU;
  CPU.loadElfFile(argv[1]);

  RISCV::EncodedInstruction encInstr;
  RISCV::DecodedInstruction decInstr;

  while (true) {
    if (CPU.getPC() == 0)
      break;

    CPU.fetch(encInstr);
    CPU.decode(encInstr, decInstr);
    CPU.execute(decInstr);
  }

  printf("\nInterpreting ELF file %s has finished\n", argv[1]);

  return 0;
}
