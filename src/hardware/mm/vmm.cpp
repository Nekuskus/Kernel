#include <stddef.h>
#include <stdint.h>
#include <hardware/mm/mm.hpp>

[[gnu::always_inline]] static inline uint8_t get_pml_index(int pml, uint64_t address) {
    return (pml >= 4 ? address & ((size_t)0x1FF << 39) : (address & ((size_t)0x1FF << (12 + (pml - 1) * 9))));
}

int map_address(uint64_t *pml4, uint16_t flags, uint64_t physical_address, size_t address) {
    uint64_t *pml3, *pml2, *pml1;
    uint8_t pml4_index = get_pml_index(4, address), pml3_index = get_pml_index(3, address),
            pml2_index = get_pml_index(2, address), pml1_index = get_pml_index(1, address);
    if (pml4[pml4_index] & 0x1)
        pml3 = (uint64_t *)((pml4[pml4_index] & 0xFFFFFFFFFFFFF000) + PHYSICAL_MEM_MAPPING);
    else {
        pml3 = (uint64_t *)((size_t)calloc(1) + PHYSICAL_MEM_MAPPING);
        if ((size_t)pml3 == PHYSICAL_MEM_MAPPING) goto fail1;
        pml4[pml4_index] = *pml3 | 0b111;
    }
    if (pml3[pml3_index] & 0x1)
        pml2 = (uint64_t *)((pml3[pml3_index] & 0xFFFFFFFFFFFFF000) + PHYSICAL_MEM_MAPPING);
    else {
        pml2 = (uint64_t *)((size_t)calloc(1) + PHYSICAL_MEM_MAPPING);
        if ((size_t)pml2 == PHYSICAL_MEM_MAPPING) goto fail2;
        pml3[pml3_index] = *pml2 | 0b111;
    }
    if (pml2[pml2_index] & 0x1)
        pml1 = (uint64_t *)((pml2[pml2_index] & 0xFFFFFFFFFFFFF000) + PHYSICAL_MEM_MAPPING);
    else {
        pml1 = (uint64_t *)((size_t)calloc(1) + PHYSICAL_MEM_MAPPING);
        if ((size_t)pml1 == PHYSICAL_MEM_MAPPING) goto fail3;
        pml2[pml2_index] = *pml1 | 0b111;
    }

    pml1[pml1_index] = physical_address | flags;
    asm volatile("invlpg (%0)" ::"r"(address)
                 : "memory");
    return 0;

fail3:
    for (size_t i = 0;; i++) {
        if (i == PAGE_TABLE_ENTRIES) {
            free((void *)(pml1 - PHYSICAL_MEM_MAPPING), 1);
            break;
        }
        if (pml1[i] & 0x1)
            goto fail1;
    }

fail2:
    for (size_t i = 0;; i++) {
        if (i == PAGE_TABLE_ENTRIES) {
            free((void *)(pml2 - PHYSICAL_MEM_MAPPING), 1);
            break;
        }
        if (pml2[i] & 0x1)
            goto fail1;
    }

fail1:
    return -1;
}

void init_vmm() {
    uint64_t *pml4 = (uint64_t *)((size_t)calloc(1) + PHYSICAL_MEM_MAPPING);
    if ((size_t)pml4 == PHYSICAL_MEM_MAPPING) return;

    for (size_t i = 0; i < (0x2000000 / PAGE_SIZE); i++) {
        size_t addr = i * PAGE_SIZE;
        map_address(pml4, addr, addr, 0x03);
        map_address(pml4, addr, PHYSICAL_MEM_MAPPING + addr, 0x03);
        map_address(pml4, addr, KERNEL_VMA + addr, 0x03 | (1 << 8));
    }

    asm volatile(
        "mov %%cr3, %%rax"
        :
        : "a"((size_t)pml4 - PHYSICAL_MEM_MAPPING));

    for (size_t i = 0; i < (0x100000000 / PAGE_SIZE); i++) {
        size_t addr = i * PAGE_SIZE;
        map_address(pml4, addr, PHYSICAL_MEM_MAPPING + addr, 0x03);
    }

    for (size_t i = 0; memory_map[i].type; i++) {
        size_t aligned_base = memory_map[i].addr - (memory_map[i].addr % PAGE_SIZE);
        size_t aligned_length = (memory_map[i].len / PAGE_SIZE) * PAGE_SIZE;
        if (memory_map[i].len % PAGE_SIZE) aligned_length += PAGE_SIZE;
        if (memory_map[i].addr % PAGE_SIZE) aligned_length += PAGE_SIZE;

        for (size_t j = 0; j * PAGE_SIZE < aligned_length; j++) {
            size_t addr = aligned_base + j * PAGE_SIZE;

            map_address(pml4, addr, PHYSICAL_MEM_MAPPING + addr, 0x03);
        }
    }
    change_alloc_method();
}