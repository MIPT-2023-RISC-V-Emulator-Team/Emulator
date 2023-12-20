#ifndef OS_HELPER_H
#define OS_HELPER_H

#include "simulator/Hart.h"
#include "simulator/memory/Memory.h"

namespace RISCV {

class OSHelper final {
private:
    static OSHelper *instancePtr;
    OSHelper() = default;

    memory::VirtAddr heapEnd_ = 0;

    bool writeMultipaged(const memory::MMU &translator,
                         const memory::VirtAddr vaddr,
                         const size_t size,
                         const uint8_t *data) const;

    bool putArgsStr(const memory::MMU &translator,
                    memory::VirtAddr virtSP,
                    int argsCount,
                    char **args,
                    std::vector<memory::VirtAddr> &argsPtr) const;
    bool putArgsPtr(const memory::MMU &translator,
                    memory::VirtAddr virtSP,
                    const std::vector<memory::VirtAddr> &argsPtr) const;

public:
    bool loadElfFile(Hart &hart, const std::string &filename);
    bool allocateStack(Hart &hart, const memory::VirtAddr stackAddr, const size_t stackSize) const;
    bool setupCmdArgs(Hart &hart, int argc, char **argv, char **envp) const;

    void handleSyscall(Hart *hart, const DecodedInstruction &instr);

    OSHelper(const OSHelper &other) = delete;

    static OSHelper *getInstance() {
        if (!instancePtr)
            instancePtr = new OSHelper();

        return instancePtr;
    }

    static void destroyInstance() {
        ASSERT(instancePtr != nullptr);
        delete instancePtr;
        instancePtr = nullptr;
    }
};

};  // namespace RISCV

#endif  // OS_HELPER_H
