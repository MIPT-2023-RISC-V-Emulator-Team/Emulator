#include "simulator/memory/MMU.h"

#include <algorithm>

namespace RISCV::memory {

// For performance aspect we store translation mode and root table physical
// address rather than SATP register itself
void MMU::setSATPReg(const RegValue satp) {
    currTransMode_ = static_cast<TranslationMode>(getPartialBitsShifted<60, 63, uint64_t>(satp));
    rootTransTablePAddr_ = getPartialBitsShifted<0, 43>(satp) * PAGE_BYTESIZE;
}

PhysAddr MMU::getPhysAddrWithAllocation(const VirtAddr vaddr, const MemoryRequest request) const {
    PhysAddr paddr;
    PhysicalMemory& pmem = getPhysicalMemory();
    PTE pte[6];

    // Since this function is called only at the beginning of the program several times
    // We can use switch twice without thinking about performance aspect
    switch (currTransMode_) {
        case TranslationMode::TRANSLATION_MODE_BARE: {
            return vaddr;
        }
        case TranslationMode::TRANSLATION_MODE_SV64: {
            break;
        }
        case TranslationMode::TRANSLATION_MODE_SV57: {
            if (!isVirtAddrCanonical<TranslationMode::TRANSLATION_MODE_SV57>(vaddr)) {
                handleException(Exception::NONCANONICAL_ADDRESS);
            }
            pte[5].setPPN(getPageNumber(rootTransTablePAddr_));
            break;
        }
        case TranslationMode::TRANSLATION_MODE_SV48: {
            if (!isVirtAddrCanonical<TranslationMode::TRANSLATION_MODE_SV48>(vaddr)) {
                handleException(Exception::NONCANONICAL_ADDRESS);
            }
            pte[4].setPPN(getPageNumber(rootTransTablePAddr_));
            break;
        }
        case TranslationMode::TRANSLATION_MODE_SV39: {
            if (!isVirtAddrCanonical<TranslationMode::TRANSLATION_MODE_SV39>(vaddr)) {
                handleException(Exception::NONCANONICAL_ADDRESS);
            }
            pte[3].setPPN(getPageNumber(rootTransTablePAddr_));
            break;
        }
    }

    // Allow fallthrough to process every page walk step by step
    switch (currTransMode_) {
        case TranslationMode::TRANSLATION_MODE_SV64: {
            uint64_t a = rootTransTablePAddr_;
            uint32_t vpn5 = getPartialBitsShifted<57, 63>(vaddr);
            PhysAddr paddr5 = a + vpn5 * PTE_SIZE;
            pmem.read(paddr5, sizeof(pte[5].value), &pte[5].value);

            if (!pte[5].getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte[5].setPPN(pageNum);
                pte[5].setAttribute(PTE::Attribute::V);

                pmem.write(paddr5, sizeof(pte[5].value), &pte[5].value);
            }
        }
        case TranslationMode::TRANSLATION_MODE_SV57: {
            uint64_t a = pte[5].getPPN() * PAGE_BYTESIZE;
            uint32_t vpn4 = getPartialBitsShifted<48, 56>(vaddr);
            PhysAddr paddr4 = a + vpn4 * PTE_SIZE;
            pmem.read(paddr4, sizeof(pte[4].value), &pte[4].value);

            if (!pte[4].getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte[4].setPPN(pageNum);
                pte[4].setAttribute(PTE::Attribute::V);

                pmem.write(paddr4, sizeof(pte[4].value), &pte[4].value);
            }
        }
        case TranslationMode::TRANSLATION_MODE_SV48: {
            uint64_t a = pte[4].getPPN() * PAGE_BYTESIZE;
            uint32_t vpn3 = getPartialBitsShifted<39, 47>(vaddr);
            PhysAddr paddr3 = a + vpn3 * PTE_SIZE;
            pmem.read(paddr3, sizeof(pte[3].value), &pte[3].value);

            if (!pte[3].getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte[3].setPPN(pageNum);
                pte[3].setAttribute(PTE::Attribute::V);

                pmem.write(paddr3, sizeof(pte[3].value), &pte[3].value);
            }
        }
        case TranslationMode::TRANSLATION_MODE_SV39: {
            uint64_t a = pte[3].getPPN() * PAGE_BYTESIZE;
            uint32_t vpn2 = getPartialBitsShifted<30, 38>(vaddr);
            PhysAddr paddr2 = a + vpn2 * PTE_SIZE;
            pmem.read(paddr2, sizeof(pte[2].value), &pte[2].value);

            if (!pte[2].getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte[2].setPPN(pageNum);
                pte[2].setAttribute(PTE::Attribute::V);

                pmem.write(paddr2, sizeof(pte[2].value), &pte[2].value);
            }

            a = pte[2].getPPN() * PAGE_BYTESIZE;
            uint32_t vpn1 = getPartialBitsShifted<21, 29>(vaddr);
            PhysAddr paddr1 = a + vpn1 * PTE_SIZE;
            pmem.read(paddr1, sizeof(pte[1].value), &pte[1].value);

            if (!pte[1].getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte[1].setPPN(pageNum);
                pte[1].setAttribute(PTE::Attribute::V);

                pmem.write(paddr1, sizeof(pte[1].value), &pte[1].value);
            }

            a = pte[1].getPPN() * PAGE_BYTESIZE;
            uint32_t vpn0 = getPartialBitsShifted<12, 20>(vaddr);
            PhysAddr paddr0 = a + vpn0 * PTE_SIZE;
            pmem.read(paddr0, sizeof(pte[0].value), &pte[0].value);

            if (!pte[0].getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage(pageNum * PAGE_BYTESIZE);

                pte[0].setPPN(pageNum);
                pte[0].setAttribute(PTE::Attribute::V);

                if (request & MemoryRequestBits::R) {
                    pte[0].setAttribute(PTE::R);
                }
                if (request & MemoryRequestBits::W) {
                    pte[0].setAttribute(PTE::W);
                }
                if (request & MemoryRequestBits::X) {
                    pte[0].setAttribute(PTE::X);
                }

                pmem.write(paddr0, sizeof(pte[0].value), &pte[0].value);
            }

            paddr = pte[0].getPPN() * PAGE_BYTESIZE;
            paddr += getPageOffset(vaddr);
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
