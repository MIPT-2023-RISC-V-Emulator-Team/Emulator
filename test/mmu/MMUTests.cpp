#include <gtest/gtest.h>
#include "simulator/memory/MMU.h"

using namespace RISCV;
using namespace RISCV::memory;

static MMU::Exception MMU_EXCEPT = MMU::Exception::NONE;

bool customExceptionHandler(const MMU::Exception exception) {
    MMU_EXCEPT = exception;
    return false;
}

#define INIT_TEST(transMode)    PhysicalMemory& pmem = getPhysicalMemory();                                 \
                                MMU mmu;                                                                    \
                                const TranslationMode satpMode = (transMode);                               \
                                const uint64_t satpAsid = 0;                                                \
                                const uint64_t satpPPN = pmem.getEmptyPageNumber();                         \
                                const RegValue satp = makePartialBits<60, 63, uint64_t>(satpMode)           \
                                                    | makePartialBits<44, 59>(satpAsid)                     \
                                                    | makePartialBits<0, 43>(satpPPN);                      \
                                mmu.setSATPReg(satp);                                                       \
                                pmem.allocatePage(satpPPN);                                                 \
                                mmu.setExceptionHandler(customExceptionHandler);


#define DEINIT_TEST()           pmem.freeAllPages();                \
                                MMU_EXCEPT = MMU::Exception::NONE;



// ====================================================== SV39 ====================================================== //

TEST(MMUTests, SV39__pte_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__pte_not_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__not_canonical_address)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0x8000000000000000;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONCANONICAL_ADDRESS);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__no_read_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__no_write_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__no_execute_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__write_no_read)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__no_leaf_pte)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__translate_1)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x3777);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__translate_2)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x3337);
    DEINIT_TEST();
}


TEST(MMUTests, SV39__translate_3)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x3ACE);
    DEINIT_TEST();
}

// ================================================================================================================== //
// ====================================================== SV48 ====================================================== //

TEST(MMUTests, SV48__pte_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__pte_not_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__not_canonical_address)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0x8000000000000000;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONCANONICAL_ADDRESS);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__no_read_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__no_write_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__no_execute_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__write_no_read)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__no_leaf_pte)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__translate_1)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x4777);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__translate_2)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x4337);
    DEINIT_TEST();
}


TEST(MMUTests, SV48__translate_3)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x4ACE);
    DEINIT_TEST();
}

// ================================================================================================================== //
// ====================================================== SV57 ====================================================== //

TEST(MMUTests, SV57__pte_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__pte_not_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__not_canonical_address)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0x8000000000000000;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONCANONICAL_ADDRESS);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__no_read_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__no_write_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__no_execute_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__write_no_read)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__no_leaf_pte)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__translate_1)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x5777);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__translate_2)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x5337);
    DEINIT_TEST();
}


TEST(MMUTests, SV57__translate_3)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x5ACE);
    DEINIT_TEST();
}

// ================================================================================================================== //
// ====================================================== SV64 ====================================================== //

TEST(MMUTests, SV64__pte_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__pte_not_valid)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__no_read_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__no_write_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__no_execute_perm)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__write_no_read)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__no_leaf_pte)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__translate_1)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x6777);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__translate_2)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x6337);
    DEINIT_TEST();
}


TEST(MMUTests, SV64__translate_3)
{
    INIT_TEST(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x6ACE);
    DEINIT_TEST();
}






int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
