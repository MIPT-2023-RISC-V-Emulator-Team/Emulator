#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "Memory.h"


namespace RISCV {

using namespace memory;

class ElfLoader {
private:
    
    static ElfLoader* instancePtr;
    ElfLoader() {};

public:
    bool loadElf(const std::string& filename, VirtAddr& outEntry);

    ElfLoader(const ElfLoader& other) = delete;

    static ElfLoader* getInstance() {
        if (!instancePtr)
            instancePtr = new ElfLoader();

        return instancePtr;
    }
};

};  // namespace RISCV

#endif  // ELF_LOADER_H
