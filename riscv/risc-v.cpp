#include <stdlib.h>

#include <chrono>

#include "simulator/OSHelper.h"
#include "simulator/Hart.h"
#include "utils/utils.h"


int main(int argc, char **argv, char **envp) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << "<elf_filename>\n";
        return -1;
    }

    RISCV::Hart CPU;
    RISCV::OSHelper* osHelper = RISCV::OSHelper::create();

    const std::string redColor("\033[0;31m");
    const std::string greenColor("\033[0;32m");
    const std::string yellowColor("\033[0;33m");
    const std::string defaultColor("\033[0m");

    // Load ELF file first
    if (!osHelper->loadElfFile(CPU, argv[1])) {
        // Fatal error
        std::cerr << redColor << "Error: could not load ELF file: " << argv[1] << defaultColor << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << greenColor << "Successfully loaded ELF file: " << argv[1] << defaultColor << std::endl;

    // Allocate stack for the process
    if (!osHelper->allocateStack(CPU, RISCV::memory::DEFAULT_STACK_ADDRESS, RISCV::memory::STACK_BYTESIZE)) {
        // Fatal error
        std::cerr << redColor << "Error: could not allocate stack" << defaultColor << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Put command arguments and environment variable onto the stack
    if (!osHelper->setupCmdArgs(CPU, argc, argv, envp)) {
        // Fatal error
        std::cerr << redColor << "Error: could not initialize command line arguments" << defaultColor << std::endl;
        std::exit(EXIT_FAILURE);
    }


    // Everything OK. Run simulation
    std::cout << "===============================================================================" << std::endl;

    int fd;
    RISCV::utils::startHostInstructionCount(&fd);

    size_t instrCount = 0;
    size_t hostInstructions = 0;
    auto executeStart = std::chrono::high_resolution_clock::now();

    // Main simulation loop
    while (CPU.getPC() != 0) {
        auto& bb = CPU.getBasicBlock();
        CPU.executeBasicBlock(bb);
        instrCount += bb.getSize();
    }

    auto executeEnd = std::chrono::high_resolution_clock::now() - executeStart;
    uint64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(executeEnd).count();

    if (fd != -1) {
        RISCV::utils::endHostInstructionCount(&fd, &hostInstructions);
    }

    std::cout << "===============================================================================" << std::endl;
    std::cout << greenColor << "Interpreting ELF file " << argv[1] << " has finished" << defaultColor << std::endl;

    std::cout << "Return value of the program: " << CPU.getReg(RISCV::RegisterType::A0) << std::endl << std::endl;

    std::cout << "Simulated instruction count: " << instrCount << std::endl;
    std::cout << "Average MIPS:                " << static_cast<float>(instrCount) / microseconds << std::endl;

    if (fd != -1) {
        std::cout << "Executed host instructions:  " << hostInstructions << std::endl;
        std::cout << "Average host per simulated:  " << (float)hostInstructions / instrCount << std::endl;
    } else {
        // Unimportant warning
        std::cerr << yellowColor << "Warning: unable to count host instructions" << defaultColor << std::endl;
    }

    RISCV::OSHelper::destroy();

    return 0;
}
