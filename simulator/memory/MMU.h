#ifndef MMU_H
#define MMU_H

#include "simulator/Cache.h"
#include "simulator/Common.h"
#include "simulator/memory/Memory.h"

namespace RISCV::memory {

struct PTE final {
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

    template <typename T>
    inline bool getAttribute(const T attr) const {
        return value & attr;
    }

    inline void setAttribute(const Attribute attr) {
        value = value | attr;
    }

    inline void resetAttribute(const Attribute attr) {
        value = value & ~attr;
    }

    inline uint64_t getPPN() const {
        return getPartialBitsShifted<10, 53>(value);
    }

    inline void setPPN(const uint64_t ppn) {
        value = (value & ~ppnMask) | makePartialBits<10, 53>(ppn);
    }

    uint64_t value;
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

    template <MemoryType type>
    ALWAYS_INLINE std::optional<uint64_t> find(const uint64_t vpn) const {
        if constexpr (type == MemoryType::IMem) {
            return iTLB_.find(vpn);
        } else if constexpr (type == MemoryType::RMem) {
            return rTLB_.find(vpn);
        }
        ASSERT(type == MemoryType::WMem);
        return wTLB_.find(vpn);
    }

    template <MemoryType type>
    ALWAYS_INLINE void insert(const uint64_t vpn, const uint64_t ppn) {
        if constexpr (type == MemoryType::IMem) {
            iTLB_.insert(vpn, ppn);
        } else if constexpr (type == MemoryType::RMem) {
            rTLB_.insert(vpn, ppn);
        } else {
            ASSERT(type == MemoryType::WMem);
            wTLB_.insert(vpn, ppn);
        }
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

    template <MemoryType type>
    ALWAYS_INLINE PhysAddr getPhysAddr(const VirtAddr vaddr) const {
        if constexpr (type == memory::MemoryType::IMem) {
            return getPhysAddr<MemoryRequestBits::R | MemoryRequestBits::X>(vaddr);
        } else if constexpr (type == memory::MemoryType::RMem) {
            return getPhysAddr<MemoryRequestBits::R>(vaddr);
        }
        ASSERT(type == memory::MemoryType::WMem);
        return getPhysAddr<MemoryRequestBits::W>(vaddr);
    }

    PhysAddr getPhysAddrWithAllocation(const VirtAddr vaddr,
                                       const MemoryRequest request = MemoryRequestBits::R |
                                                                     MemoryRequestBits::W |
                                                                     MemoryRequestBits::X) const;

private:
    // For performance aspect we store translation mode and root table physical
    // address rather than SATP register itself
    TranslationMode currTransMode_;
    uint64_t rootTransTablePAddr_;

    void handleException(const Exception exception) const;

    template<TranslationMode transMode>
    ALWAYS_INLINE bool isVirtAddrCanonical(const VirtAddr vaddr) const {
        if constexpr (transMode == TranslationMode::TRANSLATION_MODE_BARE) {
            // Always canonical for bare translation
            return true;
        }

        if constexpr (transMode == TranslationMode::TRANSLATION_MODE_SV64) {
            // Always canonical for SV64
            return true;
        }

        if constexpr (transMode == TranslationMode::TRANSLATION_MODE_SV57) {
            // Bits [63: 56]
            static constexpr const uint64_t ADDRESS_UPPER_BITS_MASK_SV57 = 0xFF00000000000000;
            const uint64_t anded = vaddr & ADDRESS_UPPER_BITS_MASK_SV57;
            if (anded == 0 || anded == ADDRESS_UPPER_BITS_MASK_SV57) {
                return true;
            }
            return false;
        }

        if constexpr (transMode == TranslationMode::TRANSLATION_MODE_SV48) {
            // Bits [63: 47]
            static constexpr const uint64_t ADDRESS_UPPER_BITS_MASK_SV48 = 0xFFFF800000000000;
            const uint64_t anded = vaddr & ADDRESS_UPPER_BITS_MASK_SV48;
            if (anded == 0 || anded == ADDRESS_UPPER_BITS_MASK_SV48) {
                return true;
            }
            return false;
        }

        if constexpr (transMode == TranslationMode::TRANSLATION_MODE_SV39) {
            // Bits [63: 38]
            static constexpr const uint64_t ADDRESS_UPPER_BITS_MASK_SV39 = 0xFFFFFFC000000000;
            const uint64_t anded = vaddr & ADDRESS_UPPER_BITS_MASK_SV39;
            if (anded == 0 || anded == ADDRESS_UPPER_BITS_MASK_SV39) {
                return true;
            }
        }

        return false;
    }

    template <MemoryRequest request>
    inline void checkPermissions(const PTE pte) const {
        if constexpr (request & MemoryRequestBits::R) {
            if (!pte.getAttribute(PTE::Attribute::R)) {
                handleException(Exception::NO_READ_PERM);
            }
        }

        if constexpr (request & MemoryRequestBits::W) {
            if (!pte.getAttribute(PTE::Attribute::W)) {
                handleException(Exception::NO_WRITE_PERM);
            }
        }

        if constexpr (request & MemoryRequestBits::X) {
            if (!pte.getAttribute(PTE::Attribute::X)) {
                handleException(Exception::NO_EXECUTE_PERM);
            }
        }
    }


    template<MemoryRequest request, uint32_t maxDepth, uint32_t currDepth = 1>
    PhysAddr pageTableWalk(const uint64_t a, const VirtAddr vaddr) const {
        constexpr uint8_t low  = 12 + (maxDepth - currDepth) * 9;
        constexpr uint8_t high = low + 8;

        uint32_t vpn = getPartialBitsShifted<low, high>(vaddr);

        PhysicalMemory& pmem = getPhysicalMemory();

        PTE pte;
        pmem.read(a + vpn * PTE_SIZE, sizeof(pte.value), &pte.value);

        if (!pte.getAttribute(PTE::Attribute::V)) {
            handleException(Exception::NOT_VALID);
        }
        if (!pte.getAttribute(PTE::Attribute::R) && pte.getAttribute(PTE::Attribute::W)) {
            handleException(Exception::WRITE_NO_READ);
        }

        if (!pte.getAttribute(PTE::Attribute::R | PTE::Attribute::X)) {
            if constexpr (currDepth == maxDepth) {
                // No leaf PTE page fault
                handleException(Exception::NO_LEAF_PTE);
            }
            else {
                // Continue page walk
                return pageTableWalk<request, maxDepth, currDepth + 1>(pte.getPPN() * PAGE_BYTESIZE, vaddr);
            }
        }
        else {
            // Found leaf pte
            PhysAddr paddr;

            if constexpr (currDepth == maxDepth) {
                // Check permissions
                checkPermissions<request>(pte);

                paddr = pte.getPPN() * PAGE_BYTESIZE;
                paddr += getPageOffset(vaddr);
                return paddr;
            }

            // Superpage
            constexpr uint64_t mask = (1 << (maxDepth - currDepth)) - 1;
            if ((pte.getPPN() & mask) != 0) {
                handleException(Exception::MISALIGNED_SUPERPAGE);
            }

            // Check permissions
            checkPermissions<request>(pte);

            paddr = ((pte.getPPN() & ~mask) | (vpn & mask)) * PAGE_BYTESIZE;
            paddr += getPageOffset(vaddr);
            return paddr;
        }
        return 0;
    }

    template <MemoryRequest request>
    PhysAddr getPhysAddr(const VirtAddr vaddr) const {
        switch (currTransMode_) {
            case TranslationMode::TRANSLATION_MODE_BARE: {
                return vaddr;
            }
            case TranslationMode::TRANSLATION_MODE_SV64: {
                return pageTableWalk<request, 6>(rootTransTablePAddr_, vaddr);
            }
            case TranslationMode::TRANSLATION_MODE_SV57: {
                if (!isVirtAddrCanonical<TranslationMode::TRANSLATION_MODE_SV57>(vaddr)) {
                    handleException(Exception::NONCANONICAL_ADDRESS);
                }
                return pageTableWalk<request, 5>(rootTransTablePAddr_, vaddr);
            }
            case TranslationMode::TRANSLATION_MODE_SV48: {
                if (!isVirtAddrCanonical<TranslationMode::TRANSLATION_MODE_SV48>(vaddr)) {
                    handleException(Exception::NONCANONICAL_ADDRESS);
                }
                return pageTableWalk<request, 4>(rootTransTablePAddr_, vaddr);
            }
            case TranslationMode::TRANSLATION_MODE_SV39: {
                if (!isVirtAddrCanonical<TranslationMode::TRANSLATION_MODE_SV39>(vaddr)) {
                    handleException(Exception::NONCANONICAL_ADDRESS);
                }
                return pageTableWalk<request, 3>(rootTransTablePAddr_, vaddr);
            }
        }

        return 0;
    }
};

}  // namespace RISCV::memory

#endif  // MMU_H
