#include <hardware/cpu/cpu.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/pmm.hpp>
#include <hardware/mm/paging.hpp>
#include <lib/math.hpp>

extern "C" Spark::Vmm::PageTable* kernel_pml4;

Spark::Vmm::PageTableEntries Spark::Vmm::virtual_to_entries(uint64_t virt) {
    Vmm::PageTableEntries off = {
        .pml4 = (virt >> 39) & 0x1ff,
        .pdp = (virt >> 30) & 0x1ff,
        .pd = (virt >> 21) & 0x1ff,
        .pt = (virt >> 12) & 0x1ff,
    };

    return off;
}

void* Spark::Vmm::entries_to_virtual(PageTableEntries offs) {
    uintptr_t addr = 0;

    addr |= offs.pml4 << 39;
    addr |= offs.pdp << 30;
    addr |= offs.pd << 21;
    addr |= offs.pt << 12;

    return (void*)addr;
}

void Spark::Vmm::init() {
    kernel_pml4 = new_address_space();
    set_context(kernel_pml4);
}

inline Spark::Vmm::PageTable* get_or_alloc_ent(Spark::Vmm::PageTable* tab, size_t off, int flags) {
    uint64_t ent_addr = tab->ents[off] & address_mask;

    if (!ent_addr) {
        ent_addr = tab->ents[off] = (uint64_t)Spark::Pmm::alloc(1);

        if (!ent_addr)
            return nullptr;

        tab->ents[off] |= flags | Spark::Vmm::VirtualMemoryFlags::VMM_PRESENT;
        memset((void*)(ent_addr + virtual_physical_base), 0, 4096);
    }

    return (Spark::Vmm::PageTable*)(ent_addr + virtual_physical_base);
}

inline Spark::Vmm::PageTable* get_or_null_ent(Spark::Vmm::PageTable* tab, size_t off) {
    uint64_t ent_addr = tab->ents[off] & address_mask;

    if (!ent_addr)
        return nullptr;

    return (Spark::Vmm::PageTable*)(ent_addr + virtual_physical_base);
}

bool Spark::Vmm::map_pages(PageTable* pml4, uint64_t virt, uint64_t phys, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_alloc_ent(pml4_virt, offs.pml4, perms);
        PageTable* pd_virt = get_or_alloc_ent(pdp_virt, offs.pdp, perms);
        PageTable* pt_virt = get_or_alloc_ent(pd_virt, offs.pd, perms);
        pt_virt->ents[offs.pt] = phys | perms;
        virt += page_size;
        phys += page_size;
    }

    return true;
}

bool Spark::Vmm::unmap_pages(PageTable* pml4, uint64_t virt, size_t count) {
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
        virt += page_size;
    }

    return true;
}

bool Spark::Vmm::update_perms(PageTable* pml4, uint64_t virt, size_t count, int perms) {
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

        pt_virt->ents[offs.pt] = (pt_virt->ents[offs.pt] & address_mask) | perms | VirtualMemoryFlags::VMM_PRESENT;
        virt += page_size;
    }

    return true;
}

bool Spark::Vmm::map_huge_pages(PageTable* pml4, uint64_t virt, uint64_t phys, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_alloc_ent(pml4_virt, offs.pml4, perms);
        PageTable* pd_virt = get_or_alloc_ent(pdp_virt, offs.pdp, perms);
        pd_virt->ents[offs.pd] = phys | perms | VirtualMemoryFlags::VMM_PRESENT | VirtualMemoryFlags::VMM_LARGE;
        virt += 0x200000;
        phys += 0x200000;
    }

    return true;
}

bool Spark::Vmm::unmap_huge_pages(PageTable* pml4, uint64_t virt, size_t count) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);
        pd_virt->ents[offs.pd] = 0;
        virt += 0x200000;
    }

    return true;
}

bool Spark::Vmm::update_huge_perms(PageTable* pml4, uint64_t virt, size_t count, int perms) {
    while (count--) {
        PageTableEntries offs = virtual_to_entries(virt);
        PageTable* pml4_virt = (PageTable*)((uint64_t)pml4 + virtual_physical_base);
        PageTable* pdp_virt = get_or_null_ent(pml4_virt, offs.pml4);

        if (!pdp_virt)
            return false;

        PageTable* pd_virt = get_or_null_ent(pdp_virt, offs.pdp);
        pd_virt->ents[offs.pd] = (pd_virt->ents[offs.pd] & address_mask) | perms | VirtualMemoryFlags::VMM_PRESENT | VirtualMemoryFlags::VMM_LARGE;
        virt += 0x200000;
    }

    return true;
}

uintptr_t Spark::Vmm::get_entry(PageTable* pml4, uint64_t virt) {
    if (virt >= 0xFFFFFFFF80000000)
        return virt - 0xFFFFFFFF80000000;

    if (virt >= 0xFFFF800000000000)
        return virt - 0xFFFF800000000000;

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

Spark::Vmm::PageTable* Spark::Vmm::new_address_space() {
    PageTable* new_pml4 = (PageTable*)Pmm::alloc(1);
    memset((void*)((uintptr_t)new_pml4 + virtual_physical_base), 0, 4096);

    map_huge_pages(new_pml4, 0xFFFFFFFF80000000, 0, 64, 3);
    map_huge_pages(new_pml4, 0xFFFF800000000000, 0, 512 * 4, 3);

    return new_pml4;
}

Spark::Vmm::PageTable** Spark::Vmm::get_ctx_ptr() {
    return (PageTable**)kernel_pml4;
}

void Spark::Vmm::save_context() {
    PageTable** ctx = get_ctx_ptr();
    *ctx = get_current_context();
}

Spark::Vmm::PageTable* get_saved_context() {
    Spark::Vmm::PageTable** ctx = Spark::Vmm::get_ctx_ptr();
    return *ctx;
}

void Spark::Vmm::restore_context() {
    PageTable** ctx = get_ctx_ptr();
    set_context(*ctx);
    *ctx = nullptr;
}

void Spark::Vmm::drop_context() {
    PageTable** ctx = get_ctx_ptr();
    *ctx = nullptr;
}

void Spark::Vmm::set_context(PageTable* ctx) {
    asm volatile("mov %%rax, %%cr3"
                 :
                 : "a"(ctx)
                 : "memory");
}

Spark::Vmm::PageTable* Spark::Vmm::get_current_context() {
    uintptr_t ctx;
    asm volatile("mov %%cr3, %%rax"
                 : "=a"(ctx)
                 :
                 : "memory");
    return (PageTable*)ctx;
}

void update_mapping(void* ptr) {
    asm volatile("invlpg (%0)"
                 :
                 : "r"(ptr)
                 : "memory");
}