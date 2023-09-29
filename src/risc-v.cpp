#include <stdlib.h>

#include <chrono>

#include "Hart.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stdout, "Usage: %s <elf_filename>\n", argv[0]);
    return -1;
  }

  RISCV::Hart CPU;
  CPU.loadElfFile(argv[1]);

  RISCV::EncodedInstruction encInstr;
  RISCV::DecodedInstruction decInstr;

  uint64_t instrCount = 0;

  uint64_t fetchMicroseconds = 0;
  uint64_t decodeMicroseconds = 0;
  uint64_t executeMicroseconds = 0;

  while (true) {
    if (CPU.getPC() == 0)
      break;

    auto fetchStart = std::chrono::high_resolution_clock::now();
    CPU.fetch(encInstr);
    auto fetchEnd = std::chrono::high_resolution_clock::now();
    fetchMicroseconds +=
        std::chrono::duration_cast<std::chrono::microseconds>(fetchEnd - fetchStart).count();

    auto decodeStart = std::chrono::high_resolution_clock::now();
    CPU.decode(encInstr, decInstr);
    auto decodeEnd = std::chrono::high_resolution_clock::now();
    decodeMicroseconds +=
        std::chrono::duration_cast<std::chrono::microseconds>(decodeEnd - decodeStart).count();

    auto executeStart = std::chrono::high_resolution_clock::now();
    CPU.execute(decInstr);
    auto executeEnd = std::chrono::high_resolution_clock::now();
    executeMicroseconds +=
        std::chrono::duration_cast<std::chrono::microseconds>(executeEnd - executeStart).count();

    ++instrCount;
  }

  printf("\nInterpreting ELF file %s has finished. Instruction count: %ld. AVG MIPS: %lf\n",
         argv[1],
         instrCount,
         static_cast<float>(instrCount) /
             (fetchMicroseconds + decodeMicroseconds + executeMicroseconds));
  printf("Fetch:    %ld ms\n", fetchMicroseconds);
  printf("Decode:   %ld ms\n", decodeMicroseconds);
  printf("Execute:  %ld ms\n", executeMicroseconds);

  return 0;
}
