#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "Hart.h"
#include "Memory.h"
#include "macros.h"

namespace RISCV {

using namespace memory;

class ElfLoader final {
private:
    static ElfLoader* instancePtr;
    ElfLoader(){};

public:
    bool loadElf(const std::string& filename, Hart& hart);

    NO_COPY_SEMANTIC(ElfLoader);
    NO_MOVE_SEMANTIC(ElfLoader);

    static ElfLoader* getInstance() {
        if (!instancePtr)
            instancePtr = new ElfLoader();

        return instancePtr;
    }
};

};  // namespace RISCV

#endif  // ELF_LOADER_H
