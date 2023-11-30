#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "simulator/Hart.h"
#include "simulator/memory/Memory.h"

namespace RISCV {

using namespace memory;

class ElfLoader final {
private:
    static ElfLoader* instancePtr;
    ElfLoader() = default;

public:
    bool loadElf(const std::string& filename, Hart& hart);

    ElfLoader(const ElfLoader& other) = delete;

    static ElfLoader* create() {
        if (!instancePtr)
            instancePtr = new ElfLoader();

        return instancePtr;
    }

    static void destroy() {
        ASSERT(instancePtr != nullptr);
        delete instancePtr;
        instancePtr = nullptr;
    }
};

};  // namespace RISCV

#endif  // ELF_LOADER_H
