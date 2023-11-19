#include "simulator/memory/MMU.h"

#include <algorithm>

namespace RISCV::memory {

// For performance aspect we store translation mode and root table physical
// address rather than SATP register itself
void MMU::setSATPReg(const RegValue satp) {
    currTransMode_ = static_cast<TranslationMode>(getPartialBitsShifted<60, 63, uint64_t>(satp));
    rootTransTablePAddr_ = getPartialBitsShifted<0, 43>(satp) * PAGE_BYTESIZE;
}

bool MMU::isVirtAddrCanonical(const VirtAddr vaddr) const {
    switch (currTransMode_) {
        case TranslationMode::TRANSLATION_MODE_BARE: {
            // Always canonical for bare translation
            return true;
        }
        case TranslationMode::TRANSLATION_MODE_SV39: {
            // Unsupported yet
            handleException(Exception::UNSUPPORTED);
            break;
        }
        case TranslationMode::TRANSLATION_MODE_SV48: {
            // Bits [63: 47]
            static constexpr const uint64_t ADDRESS_UPPER_BITS_MASK_SV48 = 0xFFFF800000000000;
            const uint64_t anded = vaddr & ADDRESS_UPPER_BITS_MASK_SV48;
            if (anded == 0 || anded == ADDRESS_UPPER_BITS_MASK_SV48) {
                return true;
            }
            return false;
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

    return false;
}

PhysAddr MMU::getPhysAddrWithAllocation(const VirtAddr vaddr, const MemoryRequest request) const {
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
            PhysAddr paddr3 = a + vpn3 * PTE_SIZE;
            pmem.read(paddr3, sizeof(pte3), &pte3);

            if (!pte3.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte3.setPPN(pageNum);
                pte3.setAttribute(PTE::Attribute::V);

                pmem.write(paddr3, sizeof(pte3), &pte3);
            }

            a = pte3.getPPN() * PAGE_BYTESIZE;
            uint32_t vpn2 = getPartialBitsShifted<30, 38>(vaddr);
            PTE pte2;
            PhysAddr paddr2 = a + vpn2 * PTE_SIZE;
            pmem.read(paddr2, sizeof(pte2), &pte2);

            if (!pte2.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte2.setPPN(pageNum);
                pte2.setAttribute(PTE::Attribute::V);

                pmem.write(paddr2, sizeof(pte2), &pte2);
            }

            a = pte2.getPPN() * PAGE_BYTESIZE;
            uint32_t vpn1 = getPartialBitsShifted<21, 29>(vaddr);
            PTE pte1;
            PhysAddr paddr1 = a + vpn1 * PTE_SIZE;
            pmem.read(paddr1, sizeof(pte1), &pte1);

            if (!pte1.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte1.setPPN(pageNum);
                pte1.setAttribute(PTE::Attribute::V);

                pmem.write(paddr1, sizeof(pte1), &pte1);
            }

            a = pte1.getPPN() * PAGE_BYTESIZE;
            uint32_t vpn0 = getPartialBitsShifted<12, 20>(vaddr);
            PTE pte0;
            PhysAddr paddr0 = a + vpn0 * PTE_SIZE;
            pmem.read(paddr0, sizeof(pte0), &pte0);

            if (!pte0.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte0.setPPN(pageNum);
                pte0.setAttribute(PTE::Attribute::V);

                if (request & MemoryRequestBits::R) {
                    pte0.setAttribute(PTE::R);
                }
                if (request & MemoryRequestBits::W) {
                    pte0.setAttribute(PTE::W);
                }
                if (request & MemoryRequestBits::X) {
                    pte0.setAttribute(PTE::X);
                }

                pmem.write(paddr0, sizeof(pte0), &pte0);
            }

            paddr = pte0.getPPN() * PAGE_BYTESIZE;
            paddr += getPageOffset(vaddr);
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

void MMU::handleException(const Exception exception) const {
    /*
     * Just print the error now, implement real handling in future
     */

    std::cerr << "MMU exception occured: ";
    switch (exception) {
        case Exception::NOT_VALID: {
            std::cerr << "invalid page table entry" << std::endl;
            break;
        }
        case Exception::WRITE_NO_READ: {
            std::cerr << "writtable page not readable" << std::endl;
            break;
        }
        case Exception::NO_READ_PERM: {
            std::cerr << "no read permission" << std::endl;
            break;
        }
        case Exception::NO_WRITE_PERM: {
            std::cerr << "no write permission" << std::endl;
            break;
        }
        case Exception::NO_EXECUTE_PERM: {
            std::cerr << "no execute permission" << std::endl;
            break;
        }
        case Exception::NO_LEAF_PTE: {
            std::cerr << "no leaf pte found" << std::endl;
            break;
        }
        case Exception::MISALIGNED_SUPERPAGE: {
            std::cerr << "misaligned superpage" << std::endl;
            break;
        }
        case Exception::NONCANONICAL_ADDRESS: {
            std::cerr << "noncanonical address" << std::endl;
            break;
        }
        case Exception::UNSUPPORTED: {
            std::cerr << "unsupported feature" << std::endl;
            break;
        }
        default: {
            std::cerr << "unknown" << std::endl;
            break;
        }
    }
    std::exit(EXIT_FAILURE);
}

}  // namespace RISCV::memory
