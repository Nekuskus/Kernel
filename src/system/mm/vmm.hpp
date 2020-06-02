#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr size_t address_mask = ~(0xFFF | (1ull << 63));

namespace Vmm {
    enum class VirtualMemoryFlags : int {
        PRESENT = 1 << 0,
        WRITE = 1 << 1,
        USER = 1 << 2,
        WT = 1 << 3,
        NO_CACHE = 1 << 4,
        DIRTY = 1 << 5,
        LARGE = 1 << 7,
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

    PageTableEntries virtual_to_entries(void *virt);
    void *entries_to_virtual(PageTableEntries entries);
    void init();
    void map_pages(PageTable *pml4, void *virt, void *phys, size_t count, int perms);
    bool unmap_pages(PageTable *pml4, void *virt, size_t count);
    bool update_perms(PageTable *pml4, void *virt, size_t count, int perms);
    bool map_huge_pages(PageTable *pml4, void *virt, void *phys, size_t count, int perms);
    bool unmap_huge_pages(PageTable *pml4, void *virt, size_t count);
    bool update_huge_perms(PageTable *pml4, void *virt, size_t count, int perms);
    uintptr_t get_entry(PageTable *pml4, void *virt);
    PageTable *new_address_space();
    PageTable *get_ctx_kernel();
    void set_context(PageTable *ctx);
    PageTable *get_current_context();
}  // namespace Vmm