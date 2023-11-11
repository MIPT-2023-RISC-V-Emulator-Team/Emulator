#ifndef MMU_H
#define MMU_H

#include "Cache.h"
#include "Common.h"
#include "Memory.h"

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

    inline bool getAttribute(const Attribute attr) const {
        return value_ & attr;
    }

    inline void setAttribute(const Attribute attr) {
        value_ = value_ | attr;
    }

    inline void resetAttribute(const Attribute attr) {
        value_ = value_ & ~attr;
    }

    inline uint64_t getPPN() const {
        return getPartialBitsShifted<10, 53>(value_);
    }

    inline void setPPN(const uint64_t ppn) {
        value_ = (value_ & ~ppnMask) | makePartialBits<10, 53>(ppn);
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

class TLB final {
public:
    static constexpr const size_t iTLB_CACHE_CAPACITY = 256;
    static constexpr const size_t rTLB_CACHE_CAPACITY = 128;
    static constexpr const size_t wTLB_CACHE_CAPACITY = 128;

    static constexpr const uint64_t INSTRUCTION_MASK = 0b1010;
    static constexpr const uint64_t READ_MASK = 0b0010;
    static constexpr const uint64_t WRITE_MASK = 0b0100;

    static constexpr const uint64_t ALL_PERMS_MASK = 0b1110;

    inline std::optional<uint64_t> findI(const uint64_t vpn) const {
        return iTLB_.find(vpn);
    }
    inline void insertI(const uint64_t vpn, const uint64_t ppn) {
        iTLB_.insert(vpn, ppn);
    }

    inline std::optional<uint64_t> findR(const uint64_t vpn) const {
        return rTLB_.find(vpn);
    }
    inline void insertR(const uint64_t vpn, const uint64_t ppn) {
        rTLB_.insert(vpn, ppn);
    }

    inline std::optional<uint64_t> findW(const uint64_t vpn) const {
        return wTLB_.find(vpn);
    }
    inline void insertW(const uint64_t vpn, const uint64_t ppn) {
        wTLB_.insert(vpn, ppn);
    }

private:
    /*
     * Approximate TLB caches with simple LRU cache
     * Use vaddr[63:12] as tag for the cache
     */
    // iTLB
    LRUCache<iTLB_CACHE_CAPACITY, uint64_t, uint64_t, false> iTLB_;

    // dTLB
    LRUCache<rTLB_CACHE_CAPACITY, uint64_t, uint64_t, false> rTLB_;
    LRUCache<wTLB_CACHE_CAPACITY, uint64_t, uint64_t, false> wTLB_;
};

class MMU final {
public:
    enum Exception : uint8_t {
        NONE = 0,

        NONCANONICAL_ADDRESS,

        NOT_VALID,
        WRITE_NO_READ,

        NO_READ_PERM,
        NO_WRITE_PERM,
        NO_EXECUTE_PERM,

        NO_LEAF_PTE,
        MISALIGNED_SUPERPAGE,

        UNSUPPORTED
    };

    void setSATPReg(const RegValue satp);
    PhysAddr getPhysAddrI(const VirtAddr vaddr) const;
    PhysAddr getPhysAddrR(const VirtAddr vaddr) const;
    PhysAddr getPhysAddrW(const VirtAddr vaddr) const;
    PhysAddr getPhysAddrWithAllocation(const VirtAddr vaddr,
                                       const MemoryRequest request = MemoryRequestBits::R |
                                                                     MemoryRequestBits::W |
                                                                     MemoryRequestBits::X) const;

private:
    // For performance aspect we store translation mode and root table physical
    // address rather than SATP register itself
    TranslationMode currTransMode_;
    uint64_t rootTransTablePAddr_;

    bool isVirtAddrCanonical(const VirtAddr vaddr) const;
    void handleException(const Exception exception) const;
};

}  // namespace RISCV::memory

#endif  // MMU_H
