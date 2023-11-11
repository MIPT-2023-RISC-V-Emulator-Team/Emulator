#include <stdlib.h>

#include <chrono>

#include "ElfLoader.h"
#include "Hart.h"

/*
 * For host instruction counting
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>

bool startHostInstructionCount(int* fd) {
    perf_event_attr pe;

    std::memset(&pe, 0, sizeof(perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    *fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (*fd == -1) {
        return false;
    }
    ioctl(*fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(*fd, PERF_EVENT_IOC_ENABLE, 0);
    return true;
}

bool endHostInstructionCount(int* fd, size_t* instructionCount) {
    ioctl(*fd, PERF_EVENT_IOC_DISABLE, 0);
    int rd = read(*fd, instructionCount, sizeof(size_t));
    bool result = true;
    if (rd == -1) {
        result = false;
    }
    close(*fd);
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stdout, "Usage: %s <elf_filename>\n", argv[0]);
        return -1;
    }

    RISCV::Hart CPU;
    RISCV::ElfLoader* elfLoader = RISCV::ElfLoader::getInstance();

    const std::string redColor("\033[0;31m");
    const std::string greenColor("\033[0;32m");
    const std::string defaultColor("\033[0m");

    if (!elfLoader->loadElf(argv[1], CPU)) {
        // Fatal error
        std::cerr << redColor << "Could not load ELF file: " << argv[1] << defaultColor
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << greenColor << "Successfully loaded ELF file: " << argv[1] << defaultColor
              << std::endl;
    std::cout << "==============================================================================="
              << std::endl;

    int fd;
    startHostInstructionCount(&fd);

    size_t instrCount = 0;
    size_t hostInstructions = 0;
    auto executeStart = std::chrono::high_resolution_clock::now();
    while (CPU.getPC() != 0) {
        const auto& bb = CPU.getBasicBlock();
        CPU.executeBasicBlock(bb);
        instrCount += bb.getSize();
    }
    auto executeEnd = std::chrono::high_resolution_clock::now() - executeStart;
    uint64_t microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(executeEnd).count();

    if (fd != -1) {
        endHostInstructionCount(&fd, &hostInstructions);
    }

    std::cout << "==============================================================================="
              << std::endl;
    std::cout << greenColor << "Interpreting ELF file " << argv[1] << " has finished\n"
              << defaultColor << std::endl;

    std::cout << "Simulated instruction count: " << instrCount << std::endl;
    std::cout << "Average MIPS:                " << static_cast<float>(instrCount) / microseconds
              << std::endl;

    if (fd != -1) {
        endHostInstructionCount(&fd, &hostInstructions);
        std::cout << "Executed host instructions:  " << hostInstructions << std::endl;
        std::cout << "Average host per simulated:  " << (float)hostInstructions / instrCount
                  << std::endl;
    } else {
        // Unimportant error
        std::cerr << redColor << "Error occured while host instruction counting" << defaultColor
                  << std::endl;
    }

    return 0;
}
