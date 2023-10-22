#include "MMU.h"

#define MMU_INLINE


namespace RISCV::memory {

void MMU::setSATPReg(RegValue* satp) {
    satpReg_ = satp;
}


PhysAddr MMU::getPhysAddr(const VirtAddr vaddr, const MemoryRequest request) const {

    PhysAddr paddr;

    const uint64_t satpMode = getPartialBitsShifted<60, 63>(*satpReg_);
    switch (satpMode) {
        case CSR_SATP_MODE_BARE: {
            paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
            paddr.pageNum = vaddr >> ADDRESS_PAGE_NUM_SHIFT;
            break;
        }
        case CSR_SATP_MODE_SV48: {
            PhysicalMemory& pmem = getPhysicalMemory();

#ifdef MMU_INLINE
            
            uint64_t a = getPartialBitsShifted<0, 43>(*satpReg_);

            uint32_t vpn3 = getPartialBitsShifted<39, 47>(vaddr);
            PTE pte3;
            pmem.read({a, vpn3 * PTE_SIZE}, sizeof(pte3), &pte3);

            
            if (pte3.getAttribute(PTE::Attribute::V) == 0) {
                handleException(Exception::NOT_VALID);
            }
            if (pte3.getAttribute(PTE::Attribute::R) == 0 && pte3.getAttribute(PTE::Attribute::W) == 1) {
                handleException(Exception::WRITE_NO_READ);
            }


            if (!pte3.getAttribute(PTE::Attribute::R) && !pte3.getAttribute(PTE::Attribute::X)) {
                a = pte3.getPPN();

                uint32_t vpn2 = getPartialBitsShifted<30, 38>(vaddr);
                PTE pte2;
                pmem.read({a, vpn2 * PTE_SIZE}, sizeof(pte2), &pte2);

                if (pte2.getAttribute(PTE::Attribute::V) == 0) {
                    handleException(Exception::NOT_VALID);
                }
                if (pte2.getAttribute(PTE::Attribute::R) == 0 && pte2.getAttribute(PTE::Attribute::W) == 1) {
                    handleException(Exception::WRITE_NO_READ);
                }


                if (!pte2.getAttribute(PTE::Attribute::R) && !pte2.getAttribute(PTE::Attribute::X)) {
                    a = pte2.getPPN();

                    uint32_t vpn1 = getPartialBitsShifted<21, 29>(vaddr);
                    PTE pte1;
                    pmem.read({a, vpn1 * PTE_SIZE}, sizeof(pte1), &pte1);          


                    if (pte1.getAttribute(PTE::Attribute::V) == 0) {
                        handleException(Exception::NOT_VALID);
                    }
                    if (pte1.getAttribute(PTE::Attribute::R) == 0 && pte1.getAttribute(PTE::Attribute::W) == 1) {
                        handleException(Exception::WRITE_NO_READ);
                    }


                    if (!pte1.getAttribute(PTE::Attribute::R) && !pte1.getAttribute(PTE::Attribute::X)) {
                        a = pte1.getPPN();

                        uint32_t vpn0 = getPartialBitsShifted<12, 20>(vaddr);
                        PTE pte0;
                        pmem.read({a, vpn0 * PTE_SIZE}, sizeof(pte0), &pte0);  


                        if (pte0.getAttribute(PTE::Attribute::V) == 0) {
                            handleException(Exception::NOT_VALID);
                        }
                        if (pte0.getAttribute(PTE::Attribute::R) == 0 && pte0.getAttribute(PTE::Attribute::W) == 1) {
                            handleException(Exception::WRITE_NO_READ);
                        }


                        if (!pte0.getAttribute(PTE::Attribute::R) && !pte0.getAttribute(PTE::Attribute::X)) {
                            // No leaf PTE page fault
                            handleException(Exception::NO_LEAF_PTE);
                        }
                        else {

                            // Check permissions
                            if (request & MemoryRequestBits::R && !pte0.getAttribute(PTE::Attribute::R)) {
                                handleException(Exception::NO_READ_PERM);
                            }
                            if (request & MemoryRequestBits::W && !pte0.getAttribute(PTE::Attribute::W)) {
                                handleException(Exception::NO_WRITE_PERM);
                            }
                            if (request & MemoryRequestBits::X && !pte0.getAttribute(PTE::Attribute::X)) {
                                handleException(Exception::NO_EXECUTE_PERM);
                            }

                            paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
                            paddr.pageNum = pte0.getPPN();
                        }
                    }
                    else {
                        // Superpage
                        uint64_t mask = 0b1;
                        if ((pte1.getPPN() & mask) != 0) {
                            handleException(Exception::MISALIGNED_SUPERPAGE);
                        }

                        // Check permissions
                        if (request & MemoryRequestBits::R && !pte1.getAttribute(PTE::Attribute::R)) {
                            handleException(Exception::NO_READ_PERM);
                        }
                        if (request & MemoryRequestBits::W && !pte1.getAttribute(PTE::Attribute::W)) {
                            handleException(Exception::NO_WRITE_PERM);
                        }
                        if (request & MemoryRequestBits::X && !pte1.getAttribute(PTE::Attribute::X)) {
                            handleException(Exception::NO_EXECUTE_PERM);
                        }

                        paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
                        paddr.pageNum = pte1.getPPN() & ~mask;
                        paddr.pageNum |= vpn1 & mask;
                    }
                }
                else {
                    // Superpage
                    uint64_t mask = 0b11;
                    if ((pte2.getPPN() & mask) != 0) {
                        handleException(Exception::MISALIGNED_SUPERPAGE);
                    }

                    // Check permissions
                    if (request & MemoryRequestBits::R && !pte2.getAttribute(PTE::Attribute::R)) {
                        handleException(Exception::NO_READ_PERM);
                    }
                    if (request & MemoryRequestBits::W && !pte2.getAttribute(PTE::Attribute::W)) {
                        handleException(Exception::NO_WRITE_PERM);
                    }
                    if (request & MemoryRequestBits::X && !pte2.getAttribute(PTE::Attribute::X)) {
                        handleException(Exception::NO_EXECUTE_PERM);
                    }

                    paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
                    paddr.pageNum = pte2.getPPN() & ~mask;
                    paddr.pageNum |= vpn2 & mask;
                }
            }
            else {
                // Superpage
                uint64_t mask = 0b111;
                if ((pte3.getPPN() & mask) != 0) {
                    handleException(Exception::MISALIGNED_SUPERPAGE);
                }

                // Check permissions
                if (request & MemoryRequestBits::R && !pte3.getAttribute(PTE::Attribute::R)) {
                    handleException(Exception::NO_READ_PERM);
                }
                if (request & MemoryRequestBits::W && !pte3.getAttribute(PTE::Attribute::W)) {
                    handleException(Exception::NO_WRITE_PERM);
                }
                if (request & MemoryRequestBits::X && !pte3.getAttribute(PTE::Attribute::X)) {
                    handleException(Exception::NO_EXECUTE_PERM);
                }

                paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
                paddr.pageNum = pte3.getPPN() & ~mask;
                paddr.pageNum |= vpn3 & mask;
            }

#else

            uint64_t vpn[PTE_LEVELS_SV48] = { 
                getPartialBitsShifted<12, 20>(vaddr),
                getPartialBitsShifted<21, 29>(vaddr),
                getPartialBitsShifted<30, 38>(vaddr),
                getPartialBitsShifted<39, 47>(vaddr)
            };

            uint64_t a = getPartialBitsShifted<0, 43>(*satpReg_);
            for (int i = PTE_LEVELS_SV48 - 1; i >= 0;) {
                PTE pte;
                pmem.read({a, (uint32_t)vpn[i] * PTE_SIZE}, sizeof(pte), &pte);

                if (pte.getAttribute(PTE::Attribute::V) == 0) {
                    handleException(Exception::NOT_VALID);
                }
                if (pte.getAttribute(PTE::Attribute::R) == 0 && pte.getAttribute(PTE::Attribute::W) == 1) {
                    handleException(Exception::WRITE_NO_READ);
                }

                if (!pte.getAttribute(PTE::Attribute::R) && !pte.getAttribute(PTE::Attribute::X)) {
                    if (i == 0) {
                        // No leaf PTE page fault
                        handleException(Exception::NO_LEAF_PTE);                        
                    }

                    --i;
                    a = pte.getPPN();
                    continue;
                }

                // Check permissions
                if (request & MemoryRequestBits::R && !pte.getAttribute(PTE::Attribute::R)) {
                    handleException(Exception::NO_READ_PERM);
                }
                if (request & MemoryRequestBits::W && !pte.getAttribute(PTE::Attribute::W)) {
                    handleException(Exception::NO_WRITE_PERM);
                }
                if (request & MemoryRequestBits::X && !pte.getAttribute(PTE::Attribute::X)) {
                    handleException(Exception::NO_EXECUTE_PERM);
                }

                paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
                paddr.pageNum = pte.getPPN();

                // Handle superpages
                if (i > 0) {
                    uint64_t mask = (0b1 << i) - 1;
                    if ((pte.getPPN() & mask) != 0) {
                        handleException(Exception::MISALIGNED_SUPERPAGE);
                    }

                    paddr.pageNum = (paddr.pageNum & ~mask) | (vpn[i] & mask);
                }
                break;
            }

#endif

            break;
        }
    }

    return paddr;
}


PhysAddr MMU::getPhysAddrWithAllocation(const VirtAddr vaddr, const MemoryRequest request) const {
    PhysAddr paddr;
    const uint64_t satpMode = getPartialBitsShifted<60, 63>(*satpReg_);

    switch (satpMode) {
        case CSR_SATP_MODE_BARE: {
            paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
            paddr.pageNum = vaddr >> ADDRESS_PAGE_NUM_SHIFT;
            break;
        }
        case CSR_SATP_MODE_SV48: {
            PhysicalMemory& pmem = getPhysicalMemory();

            uint64_t a = getPartialBitsShifted<0, 43>(*satpReg_);
            uint32_t vpn3 = getPartialBitsShifted<39, 47>(vaddr);
            PTE pte3;
            PhysAddr paddr3 = {a, vpn3 * PTE_SIZE};
            pmem.read(paddr3, sizeof(pte3), &pte3);

            if (!pte3.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage({pageNum, 0});

                pte3.setPPN(pageNum);
                pte3.setAttribute(PTE::Attribute::V);

                pmem.write(paddr3, sizeof(pte3), &pte3);
            }


            a = pte3.getPPN();
            uint32_t vpn2 = getPartialBitsShifted<30, 38>(vaddr);
            PTE pte2;
            PhysAddr paddr2 = {a, vpn2 * PTE_SIZE};
            pmem.read(paddr2, sizeof(pte2), &pte2);

            if (!pte2.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage({pageNum, 0});

                pte2.setPPN(pageNum);
                pte2.setAttribute(PTE::Attribute::V);

                pmem.write(paddr2, sizeof(pte2), &pte2);
            }


            a = pte2.getPPN();
            uint32_t vpn1 = getPartialBitsShifted<21, 29>(vaddr);
            PTE pte1;
            PhysAddr paddr1 = {a, vpn1 * PTE_SIZE};
            pmem.read(paddr1, sizeof(pte1), &pte1);

            if (!pte1.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage({pageNum, 0});

                pte1.setPPN(pageNum);
                pte1.setAttribute(PTE::Attribute::V);

                pmem.write(paddr1, sizeof(pte1), &pte1);
            }


            a = pte1.getPPN();
            uint32_t vpn0 = getPartialBitsShifted<12, 20>(vaddr);
            PTE pte0;
            PhysAddr paddr0 = {a, vpn0 * PTE_SIZE};
            pmem.read(paddr0, sizeof(pte0), &pte0);

            if (!pte0.getAttribute(PTE::Attribute::V)) {
                uint64_t pageNum = pmem.getEmptyPageNumber();
                pmem.allocatePage({pageNum, 0});

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

            paddr.pageOffset = vaddr & ADDRESS_PAGE_OFFSET_MASK;
            paddr.pageNum = pte0.getPPN();
            break;
        }
    }

    return paddr;
}


void MMU::handleException(const Exception exception) const {
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
        default: {
            std::cerr << "unknown" << std::endl;
            break;
        }
    }
    std::exit(EXIT_FAILURE);
}

}
