#include "pmm.hpp"
#include <lib/math.hpp>
#include "mm.hpp"

uint64_t* bitmap;
size_t free_pages = 0;
size_t total_pages = 0;
size_t bitmap_len = 64;

bool bit_read(size_t idx) {
    size_t off = idx / 64, mask = (1 << (idx % 64));

    return (bitmap[off] & mask) == mask;
}

void bit_write(size_t idx, int bit, size_t count) {
    for (; count; count--, idx++) {
        size_t off = idx / 64, mask = (1 << (idx % 64));

        if (bit)
            bitmap[off] |= mask;
        else
            bitmap[off] &= ~mask;
    }
}

bool bitmap_is_free(size_t idx, size_t count) {
    for (; count; idx++, count--)
        if (bit_read(idx))
            return false;

    return true;
}

uintptr_t find_available_memory_top(Firework::FireworkKernel::Multiboot::MemoryMap* mmap, size_t mmap_len) {
    uintptr_t top = 0;

    for (size_t i = 0; i < mmap_len; i++)
        if (mmap[i].type == Firework::FireworkKernel::Multiboot::MemoryState::AVAILABLE)
            if (mmap[i].addr + mmap[i].len > top)
                top = mmap[i].addr + mmap[i].len;

    return top;
}

void Firework::FireworkKernel::Pmm::init(Multiboot::MemoryMap* mmap, size_t mmap_len) {
    uintptr_t mem_top = find_available_memory_top(mmap, mmap_len);
    uint32_t mem_pages = (mem_top + page_size - 1) / page_size;
    bitmap = (uint64_t*)(memory_base + virtual_physical_base);
    bitmap_len = mem_pages;
    size_t bitmap_phys = (size_t)bitmap - virtual_physical_base;

    memset(bitmap, 0xFF, bitmap_len / 8);

    for (size_t i = 0; i < mmap_len; i++) {
        if (mmap[i].type == Multiboot::MemoryState::AVAILABLE) {
            uintptr_t start = Math::round_up(mmap[i].addr, page_size);
            size_t len = Math::round_down(mmap[i].len, page_size), count = len / page_size;

            if (!len || start + len < memory_base)
                continue;

            if (start < memory_base) {
                len -= memory_base - start;
                start = memory_base;
                count = len / page_size;
            }

            if (Math::overlaps(bitmap_phys, bitmap_len / 8, start, len)) {
                if (start < bitmap_phys)
                    free(start, (start - bitmap_phys) / page_size);

                start = bitmap_phys + bitmap_len / 8;
                len -= bitmap_len / 8;
                count = len / page_size;
            }

            free(start, count);
        }
    }

    total_pages = free_pages;
    bit_write(bitmap_phys / page_size, 1, (bitmap_len / 8 + page_size - 1) / page_size);
    alloc((bitmap_len / 8 + page_size - 1) / page_size);
}

void* Firework::FireworkKernel::Pmm::alloc(size_t count, size_t alignment, uintptr_t upper) {
    size_t idx = memory_base / page_size, max_idx = !upper ? bitmap_len : bitmap_len < (upper / page_size) ? bitmap_len : (upper / page_size);

    while (idx < max_idx) {
        if (!bitmap_is_free(idx, count)) {
            idx += alignment;
            continue;
        }

        bit_write(idx, 1, count);

        if (total_pages)
            free_pages -= count;

        return (void*)(idx * page_size);
    }

    return nullptr;
}

void* Firework::FireworkKernel::Pmm::alloc(size_t count) {
    return alloc(count, 1, 0);
}

void Firework::FireworkKernel::Pmm::free(size_t mem, size_t count) {
    size_t idx = mem / page_size;

    bit_write(idx, 0, count);

    free_pages += count;
}