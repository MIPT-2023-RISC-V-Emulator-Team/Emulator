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

    template<typename T>
    inline bool getAttribute(const T attr) const {
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

    inline PhysAddr getPhysAddrI(const VirtAddr vaddr) const {
        return getPhysAddr<MemoryRequestBits::R | MemoryRequestBits::X>(vaddr);
    }

    inline PhysAddr getPhysAddrR(const VirtAddr vaddr) const {
        return getPhysAddr<MemoryRequestBits::R>(vaddr);
    }

    inline PhysAddr getPhysAddrW(const VirtAddr vaddr) const {
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

    bool isVirtAddrCanonical(const VirtAddr vaddr) const;
    void handleException(const Exception exception) const;

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

    template <MemoryRequest request>
    PhysAddr getPhysAddr(const VirtAddr vaddr) const {
        PhysAddr paddr;

        switch (currTransMode_) {
            case TranslationMode::TRANSLATION_MODE_BARE: {
                paddr = vaddr;
                break;
            }
            case TranslationMode::TRANSLATION_MODE_SV39: {
                // Unsupported yet
                handleException(Exception::UNSUPPORTED);
                break;
            }
            case TranslationMode::TRANSLATION_MODE_SV48: {
                if (!isVirtAddrCanonical(vaddr)) {
                    handleException(Exception::NONCANONICAL_ADDRESS);
                }

                PhysicalMemory& pmem = getPhysicalMemory();

                uint64_t a = rootTransTablePAddr_;
                uint32_t vpn3 = getPartialBitsShifted<39, 47>(vaddr);
                PTE pte3;
                pmem.read(a + vpn3 * PTE_SIZE, sizeof(pte3), &pte3);

                if (!pte3.getAttribute(PTE::Attribute::V)) {
                    handleException(Exception::NOT_VALID);
                }
                if (!pte3.getAttribute(PTE::Attribute::R) && pte3.getAttribute(PTE::Attribute::W)) {
                    handleException(Exception::WRITE_NO_READ);
                }

                if (!pte3.getAttribute(PTE::Attribute::R | PTE::Attribute::X)) {

                    a = pte3.getPPN() * PAGE_BYTESIZE;
                    uint32_t vpn2 = getPartialBitsShifted<30, 38>(vaddr);
                    PTE pte2;
                    pmem.read(a + vpn2 * PTE_SIZE, sizeof(pte2), &pte2);

                    if (!pte2.getAttribute(PTE::Attribute::V)) {
                        handleException(Exception::NOT_VALID);
                    }
                    if (!pte2.getAttribute(PTE::Attribute::R) && pte2.getAttribute(PTE::Attribute::W)) {
                        handleException(Exception::WRITE_NO_READ);
                    }

                    if (!pte2.getAttribute(PTE::Attribute::R | PTE::Attribute::X)) {

                        a = pte2.getPPN() * PAGE_BYTESIZE;
                        uint32_t vpn1 = getPartialBitsShifted<21, 29>(vaddr);
                        PTE pte1;
                        pmem.read(a + vpn1 * PTE_SIZE, sizeof(pte1), &pte1);

                        if (!pte1.getAttribute(PTE::Attribute::V)) {
                            handleException(Exception::NOT_VALID);
                        }
                        if (!pte1.getAttribute(PTE::Attribute::R) && pte1.getAttribute(PTE::Attribute::W)) {
                            handleException(Exception::WRITE_NO_READ);
                        }

                        if (!pte1.getAttribute(PTE::Attribute::R | PTE::Attribute::X)) {

                            a = pte1.getPPN() * PAGE_BYTESIZE;
                            uint32_t vpn0 = getPartialBitsShifted<12, 20>(vaddr);
                            PTE pte0;
                            pmem.read(a + vpn0 * PTE_SIZE, sizeof(pte0), &pte0);

                            if (!pte0.getAttribute(PTE::Attribute::V)) {
                                handleException(Exception::NOT_VALID);
                            }
                            if (!pte0.getAttribute(PTE::Attribute::R) && pte0.getAttribute(PTE::Attribute::W)) {
                                handleException(Exception::WRITE_NO_READ);
                            }

                            if (!pte0.getAttribute(PTE::Attribute::R | PTE::Attribute::X)) {
                                // No leaf PTE page fault
                                handleException(Exception::NO_LEAF_PTE);
                            } else {
                                // Check permissions
                                checkPermissions<request>(pte0);

                                paddr = pte0.getPPN() * PAGE_BYTESIZE;
                                paddr += getPageOffset(vaddr);
                            }
                        } else {
                            // Superpage
                            uint64_t mask = 0b1;
                            if ((pte1.getPPN() & mask) != 0) {
                                handleException(Exception::MISALIGNED_SUPERPAGE);
                            }

                            // Check permissions
                            checkPermissions<request>(pte1);

                            paddr = ((pte1.getPPN() & ~mask) | (vpn1 & mask)) * PAGE_BYTESIZE;
                            paddr += getPageOffset(vaddr);
                        }
                    } else {
                        // Superpage
                        uint64_t mask = 0b11;
                        if ((pte2.getPPN() & mask) != 0) {
                            handleException(Exception::MISALIGNED_SUPERPAGE);
                        }

                        // Check permissions
                        checkPermissions<request>(pte2);

                        paddr = ((pte2.getPPN() & ~mask) | (vpn2 & mask)) * PAGE_BYTESIZE;
                        paddr += getPageOffset(vaddr);
                    }
                } else {
                    // Superpage
                    uint64_t mask = 0b111;
                    if ((pte3.getPPN() & mask) != 0) {
                        handleException(Exception::MISALIGNED_SUPERPAGE);
                    }

                    // Check permissions
                    checkPermissions<request>(pte3);

                    paddr = ((pte3.getPPN() & ~mask) | (vpn3 & mask)) * PAGE_BYTESIZE;
                    paddr += getPageOffset(vaddr);
                }
                break;
            }
            case TranslationMode::TRANSLATION_MODE_SV57: {
                // Unsupported yet
                handleException(Exception::UNSUPPORTED);
                break;
            }
            case TranslationMode::TRANSLATION_MODE_SV64: {
                // Unsupported yet
                handleException(Exception::UNSUPPORTED);
                break;
            }
        }

        return paddr;
    }
};

}  // namespace RISCV::memory

#endif  // MMU_H
