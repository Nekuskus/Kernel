#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr size_t address_mask = ~(0xFFF | (1ull << 63));

namespace Firework::FireworkKernel::Vmm {
    enum VirtualMemoryFlags {
        VMM_PRESENT = 1 << 0,
        VMM_WRITE = 1 << 1,
        VMM_USER = 1 << 2,
        VMM_WT = 1 << 3,
        VMM_NO_CACHE = 1 << 4,
        VMM_DIRTY = 1 << 5,
        VMM_LARGE = 1 << 7,
    };

    struct PageTable {
        uint64_t ents[512];
    };

    struct PageTableEntries {
        size_t pml4;
        size_t pdp;
        size_t pd;
        size_t pt;
    };

    PageTableEntries virtual_to_entries(uint64_t virt);
    void* entries_to_virtual(PageTableEntries entries);
    void init();
    bool map_pages(PageTable* pml4, uint64_t virt, uint64_t phys, size_t count, int perms);
    bool unmap_pages(PageTable* pml4, uint64_t virt, size_t count);
    bool update_perms(PageTable* pml4, uint64_t virt, size_t count, int perms);
    bool map_huge_pages(PageTable* pml4, uint64_t virt, uint64_t phys, size_t count, int perms);
    bool unmap_huge_pages(PageTable* pml4, uint64_t virt, size_t count);
    bool update_huge_perms(PageTable* pml4, uint64_t virt, size_t count, int perms);
    uintptr_t get_entry(PageTable* pml4, uint64_t virt);
    PageTable* new_address_space();
    void save_context();
    PageTable** get_ctx_ptr();
    void restore_context();
    void drop_context();
    void set_context(PageTable* ctx);
    PageTable* get_current_context();
}  // namespace Firework::FireworkKernel::Vmm