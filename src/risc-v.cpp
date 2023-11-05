#include <stdlib.h>

#include <chrono>

#include "Hart.h"
#include "ElfLoader.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stdout, "Usage: %s <elf_filename>\n", argv[0]);
        return -1;
    }

    RISCV::Hart CPU;
    RISCV::ElfLoader* elfLoader = RISCV::ElfLoader::getInstance();

    elfLoader->loadElf(argv[1], CPU);

    uint64_t instrCount = 0;

    auto executeStart = std::chrono::high_resolution_clock::now();
    while (true) {
        if (CPU.getPC() == 0) {
            break;
        }

        auto encInstr = CPU.fetch();
        auto decInstr = CPU.decode(encInstr);
        CPU.execute(decInstr);

        ++instrCount;
    }
    auto executeEnd = std::chrono::high_resolution_clock::now() - executeStart;

    printf("\nInterpreting ELF file %s has finished. Instruction count: %ld. AVG MIPS: %lf\n",
           argv[1],
           instrCount,
           static_cast<float>(instrCount) /
               (std::chrono::duration_cast<std::chrono::microseconds>(executeEnd).count()));

    return 0;
}
