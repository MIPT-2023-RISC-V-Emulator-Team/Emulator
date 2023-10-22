#ifndef MMU_H
#define MMU_H

#include "Memory.h"
#include "Common.h"


namespace RISCV::memory {

class PTE final {
public:
    enum Attribute : uint64_t {
        V = 1 << 0,
        R = 1 << 1,
        W = 1 << 2,
        X = 1 << 3,
        U = 1 << 4,
        G = 1 << 5,
        A = 1 << 6,
        D = 1 << 7
    };

    static constexpr const uint64_t ppnMask = 0x3FFFFFFFFFFC00ULL;

    bool getAttribute(const Attribute attr) {
        return value_ & attr;
    }

    void setAttribute(const Attribute attr) {
        value_ = value_ | attr;
    }

    void resetAttribute(const Attribute attr) {
        value_ = value_ & ~attr;
    }

    uint64_t getPPN() {
        return getPartialBitsShifted<10, 53, uint64_t>(value_);
    }

    void setPPN(const uint64_t ppn) {
        value_ = (value_ & ~ppnMask) | makePartialBits<10, 53, uint64_t>(ppn);
    }

private:
    uint64_t value_;
};


using MemoryRequest = uint8_t;

enum MemoryRequestBits : MemoryRequest {
    R = PTE::Attribute::R,
    W = PTE::Attribute::W,
    X = PTE::Attribute::X
};


class MMU final {
public:
    enum Exception : uint8_t {
        NONE = 0,
        
        NOT_VALID,
        WRITE_NO_READ,

        NO_READ_PERM,
        NO_WRITE_PERM,
        NO_EXECUTE_PERM,

        NO_LEAF_PTE,
        MISALIGNED_SUPERPAGE,

        UNSUPPORTED
    };

    void setSATPReg(RegValue* satp);
    PhysAddr getPhysAddr(const VirtAddr vaddr, const MemoryRequest request = MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X) const;
    PhysAddr getPhysAddrWithAllocation(const VirtAddr vaddr, const MemoryRequest request = MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X) const;

private:
    RegValue* satpReg_ = nullptr;
    
    void handleException(const Exception exception) const;
};

}

#endif // MMU_H
