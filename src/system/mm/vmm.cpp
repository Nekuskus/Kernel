#include "vmm.hpp"

#include <lib/lib.hpp>

#include "mm.hpp"
#include "pmm.hpp"

extern "C" Vmm::PageTable* kernel_pml4;

Vmm::PageTableEntries Vmm::virtual_to_entries(void* virt) {
    PageTableEntries off = {
        .pml4 = ((size_t)virt >> 39) & 0x1ff,
        .pdp = ((size_t)virt >> 30) & 0x1ff,
        .pd = ((size_t)virt >> 21) & 0x1ff,
        .pt = ((size_t)virt >> 12) & 0x1ff,
    };

    return off;
}

void* Vmm::entries_to_virtual(PageTableEntries offs) {
    uintptr_t addr = 0;

    addr |= offs.pml4 << 39;
    addr |= offs.pdp << 30;
    addr |= offs.pd << 21;
    addr |= offs.pt << 12;

    return (void*)addr;
}

void Vmm::init() {
    kernel_pml4 = new_address_space();

    set_context(kernel_pml4);
}

Vmm::PageTable* get_or_alloc_ent(Vmm::PageTable* tab, size_t off, int flags) {
    uint64_t ent_addr = tab->ents[off] & address_mask;

    if (!ent_addr) {
        ent_addr = tab->ents[off] = (uint64_t)Pmm::alloc(1);

        if (!ent_addr)
            return nullptr;

        tab->ents[off] |= flags | Vmm::VirtualMemoryFlags::VMM_PRESENT;
        memset((void*)(ent_addr + virtual_physical_base), 0, 4096);
    }

    return (Vmm::PageTable*)(ent_addr + virtual_physical_base);
}

Vmm::PageTable* get_or_null_ent(Vmm::PageTable* tab, size_t off) {
    uint64_t ent_addr = tab->ents[off] & address_mask;

    if (!ent_addr)
        return nullptr;

    return (Vmm::PageTable*)(ent_addr + virtual_physical_base);
}

bool Vmm::map_pages(PageTable* pml4, void* virt, void* phys, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_alloc_ent(pml4_virt, offs.pml4, perms);
        PageTable* pd_virt = get_or_alloc_ent(pdp_virt, offs.pdp, perms);
        PageTable* pt_virt = get_or_alloc_ent(pd_virt, offs.pd, perms);
        pt_virt->ents[offs.pt] = (uint64_t)phys | perms;
        virt = (void*)((uintptr_t)virt + page_size);
        phys = (void*)((uintptr_t)phys + page_size);
    }

    return true;
}

void update_mapping(void* ptr) {
    asm volatile("invlpg (%0)"
                 :
                 : "r"(ptr)
                 : "memory");
}

bool Vmm::unmap_pages(PageTable* pml4, void* virt, size_t count) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);

        if (!pd_virt)
            return false;

        PageTable* pt_virt = get_or_null_ent(pd_virt, offs.pd);

        if (!pt_virt)
            return false;

        pt_virt->ents[offs.pt] = 0;
        update_mapping(virt);
        virt = (void*)((uintptr_t)virt + page_size);
    }

    return true;
}

bool Vmm::update_perms(PageTable* pml4, void* virt, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);

        if (!pd_virt)
            return false;

        PageTable* pt_virt = get_or_null_ent(pd_virt, offs.pd);

        if (!pt_virt)
            return false;

        pt_virt->ents[offs.pt] = (pt_virt->ents[offs.pt] & address_mask) | perms;
        virt = (void*)((uintptr_t)virt + page_size);
    }

    return true;
}

bool Vmm::map_huge_pages(PageTable* pml4, void* virt, void* phys, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_alloc_ent(pml4_virt, offs.pml4, perms);
        PageTable* pd_virt = get_or_alloc_ent(pdp_virt, offs.pdp, perms);
        pd_virt->ents[offs.pd] = (uint64_t)phys | perms | VirtualMemoryFlags::VMM_LARGE;
        virt = (void*)((uintptr_t)virt + 0x200000);
        phys = (void*)((uintptr_t)phys + 0x200000);
    }

    return true;
}

bool Vmm::unmap_huge_pages(PageTable* pml4, void* virt, size_t count) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);
        pd_virt->ents[offs.pd] = 0;
        update_mapping(virt);
        virt = (void*)((uintptr_t)virt + 0x200000);
    }

    return true;
}

bool Vmm::update_huge_perms(PageTable* pml4, void* virt, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);
        pd_virt->ents[offs.pd] = (pd_virt->ents[offs.pd] & address_mask) | perms | VirtualMemoryFlags::VMM_LARGE;
        virt = (void*)((uintptr_t)virt + 0x200000);
    }

    return true;
}

uintptr_t Vmm::get_entry(PageTable* pml4, void* virt) {
    PageTableEntries offs = virtual_to_entries(virt);
    PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
    PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

    if (!pdp_virt)
        return 0;

    PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);

    if (!pd_virt)
        return 0;

    PageTable* pt_virt = get_or_null_ent(pd_virt, offs.pd);

    if (!pt_virt)
        return 0;

    return pt_virt->ents[offs.pt];
}

Vmm::PageTable* Vmm::new_address_space() {
    PageTable* new_pml4 = (PageTable*)Pmm::alloc(1);

    memset((void*)((uintptr_t)new_pml4 + virtual_physical_base), 0, 4096);
    map_huge_pages(new_pml4, (void*)0xFFFFFFFF80000000, NULL, 64, 3);
    map_huge_pages(new_pml4, (void*)0xFFFF800000000000, NULL, 512 * 4, 3);

    return new_pml4;
}

Vmm::PageTable** Vmm::get_ctx_ptr() {
    return (PageTable**)kernel_pml4;
}

Vmm::PageTable* Vmm::get_ctx_kernel() {
    return kernel_pml4;
}

void Vmm::save_context() {
    PageTable** ctx = get_ctx_ptr();
    *ctx = get_current_context();
}

Vmm::PageTable* get_saved_context() {
    Vmm::PageTable** ctx = Vmm::get_ctx_ptr();
    return *ctx;
}

void Vmm::restore_context() {
    PageTable** ctx = get_ctx_ptr();
    set_context(*ctx);
    *ctx = nullptr;
}

void Vmm::drop_context() {
    PageTable** ctx = get_ctx_ptr();
    *ctx = nullptr;
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