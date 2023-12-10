#include <gtest/gtest.h>

#include "simulator/memory/MMU.h"

using namespace RISCV;
using namespace RISCV::memory;

static PhysicalMemory &pmem = getPhysicalMemory();
static MMU mmu;
static MMU::Exception MMU_EXCEPT = MMU::Exception::NONE;

bool customExceptionHandler(const MMU::Exception exception) {
    MMU_EXCEPT = exception;
    return false;
}

class MMUTest : public testing::Test {
public:
    void SetUp() override {
        mmu.setExceptionHandler(customExceptionHandler);
    }

    void TearDown() override {
        pmem.freeAllPages();
        MMU_EXCEPT = MMU::Exception::NONE;
    }

    void SetTranslationMode(TranslationMode transMode) {
        const uint64_t satpAsid = 0;
        const uint64_t satpPPN = pmem.getEmptyPageNumber();
        const RegValue satp = makePartialBits<60, 63, uint64_t>(transMode) | makePartialBits<44, 59>(satpAsid) |
                              makePartialBits<0, 43>(satpPPN);
        mmu.setSATPReg(satp);
        pmem.allocatePage(satpPPN);
    }

    MMUTest() = default;
    ~MMUTest() override = default;

    NO_MOVE_SEMANTIC(MMUTest);
    NO_COPY_SEMANTIC(MMUTest);
};

// ====================================================== SV39 ====================================================== //

TEST_F(MMUTest, SV39__pte_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
}

TEST_F(MMUTest, SV39__pte_not_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
}

TEST_F(MMUTest, SV39__not_canonical_address) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0x8000000000000000;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONCANONICAL_ADDRESS);
}

TEST_F(MMUTest, SV39__no_read_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
}

TEST_F(MMUTest, SV39__no_write_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
}

TEST_F(MMUTest, SV39__no_execute_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
}

TEST_F(MMUTest, SV39__write_no_read) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
}

TEST_F(MMUTest, SV39__no_leaf_pte) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
}

TEST_F(MMUTest, SV39__translate_1) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x3777);
}

TEST_F(MMUTest, SV39__translate_2) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x3337);
}

TEST_F(MMUTest, SV39__translate_3) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV39);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x3ACE);
}

// ================================================================================================================== //
// ====================================================== SV48 ====================================================== //

TEST_F(MMUTest, SV48__pte_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
}

TEST_F(MMUTest, SV48__pte_not_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
}

TEST_F(MMUTest, SV48__not_canonical_address) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0x8000000000000000;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONCANONICAL_ADDRESS);
}

TEST_F(MMUTest, SV48__no_read_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
}

TEST_F(MMUTest, SV48__no_write_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
}

TEST_F(MMUTest, SV48__no_execute_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
}

TEST_F(MMUTest, SV48__write_no_read) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
}

TEST_F(MMUTest, SV48__no_leaf_pte) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
}

TEST_F(MMUTest, SV48__translate_1) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x4777);
}

TEST_F(MMUTest, SV48__translate_2) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x4337);
}

TEST_F(MMUTest, SV48__translate_3) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV48);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x4ACE);
}

// ================================================================================================================== //
// ====================================================== SV57 ====================================================== //

TEST_F(MMUTest, SV57__pte_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
}

TEST_F(MMUTest, SV57__pte_not_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
}

TEST_F(MMUTest, SV57__not_canonical_address) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0x8000000000000000;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONCANONICAL_ADDRESS);
}

TEST_F(MMUTest, SV57__no_read_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
}

TEST_F(MMUTest, SV57__no_write_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
}

TEST_F(MMUTest, SV57__no_execute_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
}

TEST_F(MMUTest, SV57__write_no_read) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
}

TEST_F(MMUTest, SV57__no_leaf_pte) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
}

TEST_F(MMUTest, SV57__translate_1) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x5777);
}

TEST_F(MMUTest, SV57__translate_2) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x5337);
}

TEST_F(MMUTest, SV57__translate_3) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV57);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x5ACE);
}

// ================================================================================================================== //
// ====================================================== SV64 ====================================================== //

TEST_F(MMUTest, SV64__pte_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NONE);
}

TEST_F(MMUTest, SV64__pte_not_valid) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 =
        mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R | MemoryRequestBits::W | MemoryRequestBits::X);

    VirtAddr vaddr2 = 0xFFF777;
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr2);

    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::PTE_NOT_VALID);
}

TEST_F(MMUTest, SV64__no_read_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::X);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_READ_PERM);
}

TEST_F(MMUTest, SV64__no_write_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_WRITE_PERM);
}

TEST_F(MMUTest, SV64__no_execute_perm) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::R);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::IMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_EXECUTE_PERM);
}

TEST_F(MMUTest, SV64__write_no_read) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, MemoryRequestBits::W);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::WMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::WRITE_NO_READ);
}

TEST_F(MMUTest, SV64__no_leaf_pte) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr1 = 0xFFFFF7777;
    PhysAddr paddr1 = mmu.getPhysAddrWithAllocation(vaddr1, 0);
    PhysAddr paddr2 = mmu.getPhysAddr<MemoryType::RMem>(vaddr1);
    ASSERT_EQ(MMU_EXCEPT, MMU::Exception::NO_LEAF_PTE);
}

TEST_F(MMUTest, SV64__translate_1) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr = 0xFFFFF7777;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x6777);
}

TEST_F(MMUTest, SV64__translate_2) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr = 0xABC42337;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x6337);
}

TEST_F(MMUTest, SV64__translate_3) {
    SetTranslationMode(TranslationMode::TRANSLATION_MODE_SV64);

    VirtAddr vaddr = 0x02468ACE;
    PhysAddr paddr = mmu.getPhysAddrWithAllocation(vaddr);
    // Manualy calculated
    ASSERT_EQ(paddr, 0x6ACE);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
