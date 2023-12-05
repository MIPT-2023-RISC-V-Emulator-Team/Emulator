#ifndef OS_HELPER_H
#define OS_HELPER_H

#include "simulator/Hart.h"
#include "simulator/memory/Memory.h"

namespace RISCV {

class OSHelper final {
private:
    static OSHelper *instancePtr;
    OSHelper() = default;

public:
    bool loadElfFile(Hart &hart, const std::string &filename);
    bool allocateStack(Hart &hart, const memory::VirtAddr stackAddr, const size_t stackSize);
    bool setupCmdArgs(Hart &hart, int argc, char **argv, char **envp);

    OSHelper(const OSHelper &other) = delete;

    static OSHelper *create() {
        if (!instancePtr)
            instancePtr = new OSHelper();

        return instancePtr;
    }

    static void destroy() {
        ASSERT(instancePtr != nullptr);
        delete instancePtr;
        instancePtr = nullptr;
    }
};

};  // namespace RISCV

#endif  // OS_HELPER_H
