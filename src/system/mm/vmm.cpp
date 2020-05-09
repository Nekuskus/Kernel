#include "vmm.hpp"

#include <lib/lib.hpp>

#include "mm.hpp"
#include "pmm.hpp"

extern "C" Vmm::PageTable* kernel_pml4;

inline Vmm::PageTableEntries Vmm::virtual_to_entries(void* virt) {
    PageTableEntries off = {
        .pml4 = ((uint64_t)virt >> 39) & 0x1ff,
        .pdp = ((uint64_t)virt >> 30) & 0x1ff,
        .pd = ((uint64_t)virt >> 21) & 0x1ff,
        .pt = ((uint64_t)virt >> 12) & 0x1ff,
    };

    return off;
}

void* Vmm::entries_to_virtual(PageTableEntries offs) {
    return (void*)((offs.pml4 << 39) | (offs.pdp << 30) | (offs.pd << 21) | (offs.pt << 12));
}

void Vmm::init() {
    kernel_pml4 = new_address_space();

    set_context(kernel_pml4);
}

inline Vmm::PageTable* get_or_alloc_ent(Vmm::PageTable* tab, size_t off, int flags) {
    uint64_t ent_addr = tab->ents[off] & address_mask;

    if (!ent_addr) {
        ent_addr = tab->ents[off] = (uint64_t)Pmm::alloc(1);

        tab->ents[off] |= flags | Vmm::VirtualMemoryFlags::VMM_PRESENT;
        memset((void*)(ent_addr + virtual_physical_base), 0, 4096);
    }

    return (Vmm::PageTable*)(ent_addr + virtual_physical_base);
}

inline Vmm::PageTable* get_or_null_ent(Vmm::PageTable* tab, size_t off) {
    uint64_t ent_addr = tab->ents[off] & address_mask;

    return !ent_addr ? nullptr : (Vmm::PageTable*)(ent_addr + virtual_physical_base);
}

void Vmm::map_pages(PageTable* pml4, void* virt, void* phys, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_alloc_ent(pml4_virt, offs.pml4, perms);
        PageTable* pd_virt = get_or_alloc_ent(pdp_virt, offs.pdp, perms);
        PageTable* pt_virt = get_or_alloc_ent(pd_virt, offs.pd, perms);
        pt_virt->ents[offs.pt] = (uint64_t)phys | perms;
        virt = (void*)((uint64_t)virt + 0x1000);
        phys = (void*)((uint64_t)phys + 0x1000);
    }
}

void update_mapping(void* ptr) {
    asm volatile("invlpg (%0)"
                 :
                 : "r"(ptr)
                 : "memory");
}

bool Vmm::unmap_pages(PageTable* pml4, void* virt, size_t count) {
    while (count--) {
        auto offs = virtual_to_entries(virt);
        auto pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        auto pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        auto pd_virt = get_or_null_ent(pdp_virt, offs.pdp);

        if (!pd_virt)
            return false;

        auto pt_virt = get_or_null_ent(pd_virt, offs.pd);

        if (!pt_virt)
            return false;

        pt_virt->ents[offs.pt] = 0;
        update_mapping(virt);
        virt = (void*)((uint64_t)virt + 0x1000);
    }

    return true;
}

bool Vmm::update_perms(PageTable* pml4, void* virt, size_t count, int perms) {
    while (count--) {
        auto offs = virtual_to_entries(virt);
        auto pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        auto pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        auto pd_virt = get_or_null_ent(pdp_virt, offs.pdp);

        if (!pd_virt)
            return false;

        auto pt_virt = get_or_null_ent(pd_virt, offs.pd);

        if (!pt_virt)
            return false;

        pt_virt->ents[offs.pt] = (pt_virt->ents[offs.pt] & address_mask) | perms;
        virt = (void*)((uint64_t)virt + 0x1000);
    }

    return true;
}

bool Vmm::map_huge_pages(PageTable* pml4, void* virt, void* phys, size_t count, int perms) {
    while (count--) {
        auto offs = virtual_to_entries(virt);
        auto pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        auto pdp_virt = get_or_alloc_ent(pml4_virt, offs.pml4, perms);
        auto pd_virt = get_or_alloc_ent(pdp_virt, offs.pdp, perms);
        pd_virt->ents[offs.pd] = (uint64_t)phys | perms | VirtualMemoryFlags::VMM_LARGE;
        virt = (void*)((uint64_t)virt + 0x200000);
        phys = (void*)((uint64_t)phys + 0x200000);
    }

    return true;
}

bool Vmm::unmap_huge_pages(PageTable* pml4, void* virt, size_t count) {
    while (count--) {
        auto offs = virtual_to_entries(virt);
        auto pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        auto pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        auto pd_virt = get_or_null_ent(pdp_virt, offs.pdp);
        pd_virt->ents[offs.pd] = 0;
        update_mapping(virt);
        virt = (void*)((uint64_t)virt + 0x200000);
    }

    return true;
}

bool Vmm::update_huge_perms(PageTable* pml4, void* virt, size_t count, int perms) {
    while (count--) {
        auto offs = virtual_to_entries(virt);
        auto pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        auto pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        auto pd_virt = get_or_null_ent(pdp_virt, offs.pdp);
        pd_virt->ents[offs.pd] = (pd_virt->ents[offs.pd] & address_mask) | perms | VirtualMemoryFlags::VMM_LARGE;
        virt = (void*)((uint64_t)virt + 0x200000);
    }

    return true;
}

uintptr_t Vmm::get_entry(PageTable* pml4, void* virt) {
    auto offs = virtual_to_entries(virt);
    auto pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
    auto pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

    if (!pdp_virt)
        return 0;

    auto pd_virt = get_or_null_ent(pdp_virt, offs.pdp);

    if (!pd_virt)
        return 0;

    auto pt_virt = get_or_null_ent(pd_virt, offs.pd);

    if (!pt_virt)
        return 0;

    return pt_virt->ents[offs.pt];
}

Vmm::PageTable* Vmm::new_address_space() {
    auto new_pml4 = (PageTable*)Pmm::alloc(1);

    memset((void*)((uint64_t)new_pml4 + virtual_physical_base), 0, 4096);
    map_huge_pages(new_pml4, (void*)0xFFFFFFFF80000000, NULL, 64, VirtualMemoryFlags::VMM_PRESENT | VirtualMemoryFlags::VMM_WRITE);
    map_huge_pages(new_pml4, (void*)0xFFFF800000000000, NULL, 512 * 4, VirtualMemoryFlags::VMM_PRESENT | VirtualMemoryFlags::VMM_WRITE);

    return new_pml4;
}

Vmm::PageTable* Vmm::get_ctx_kernel() {
    return kernel_pml4;
}

void Vmm::set_context(PageTable* ctx) {
    asm volatile("mov %%rax, %%cr3"
                 :
                 : "a"(ctx)
                 : "memory");
}

Vmm::PageTable* Vmm::get_current_context() {
    uintptr_t ctx;
    asm volatile("mov %%cr3, %%rax"
                 : "=a"(ctx)
                 :
                 : "memory");
    return (PageTable*)ctx;
}