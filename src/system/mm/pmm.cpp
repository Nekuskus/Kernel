#include "pmm.hpp"

#include <lib/lib.hpp>
#include <lib/math.hpp>
#include <system/panic.hpp>

#include "mm.hpp"

static uint64_t* bitmap;
static size_t bitmap_len = 64;
static size_t free_pages = 0;
static size_t total_pages = 0;
static size_t cur_idx = 0;

extern "C" void* _kernel_start;
extern "C" void* _kernel_end;
extern "C" uint64_t trampoline_size;

bool bit_read(size_t idx) {
    size_t off = idx / 64, mask = 1UL << (idx % 64UL);

    return (bitmap[off] & mask) == mask;
}

void bit_write(size_t idx, int bit, size_t count) {
    for (; count; count--, idx++) {
        size_t off = idx / 64, mask = 1UL << (idx % 64UL);

        bit ? bitmap[off] |= mask : bitmap[off] &= ~mask;
    }
}

bool bitmap_is_free(size_t idx, size_t count) {
    for (; count; idx++, count--)
        if (bit_read(idx))
            return false;

    return true;
}

uintptr_t find_available_memory_top(Stivale::StivaleMMapEntry* mmap, size_t mmap_len) {
    uintptr_t top = 0;

    for (size_t i = 0; i < mmap_len; i++)
        if (mmap[i].type == Stivale::StivaleMMapType::USABLE_RAM)
            if (uint64_t entry_end = mmap[i].base + mmap[i].length; entry_end > top)
                top = entry_end;

    return top;
}

void Pmm::init(Stivale::StivaleMMapEntry* mmap, size_t mmap_len) {
    auto mem_top = find_available_memory_top(mmap, mmap_len);
    uint32_t mem_pages = (mem_top + 0x1000 - 1) / 0x1000;
    bitmap = (uint64_t*)(memory_base + virtual_physical_base);
    bitmap_len = mem_pages;
    size_t bitmap_phys = (size_t)bitmap - virtual_physical_base;

    memset(bitmap, 0xFF, bitmap_len / 8);

    for (size_t i = 0; i < mmap_len; i++) {
        if (mmap[i].type == Stivale::StivaleMMapType::USABLE_RAM) {
            uintptr_t start = Math::round_up(mmap[i].base, 0x1000);
            size_t len = Math::round_down(mmap[i].length, 0x1000), count = len / 0x1000;

            if (!len || start + len < memory_base)
                continue;

            if (start < memory_base) {
                len -= memory_base - start;
                start = memory_base;
                count = len / 0x1000;
            }

            if (Math::overlaps(bitmap_phys, bitmap_len / 8, start, len)) {
                if (start < bitmap_phys)
                    free((void*)start, (start - bitmap_phys) / 0x1000);

                start = bitmap_phys + bitmap_len / 8;
                len -= bitmap_len / 8;
                count = len / 0x1000;
            }

            free((void*)start, count);
        }
    }

    bit_write((size_t)(&_kernel_start - virtual_kernel_base) / 0x1000, 1, ((size_t)(&_kernel_end - &_kernel_start) + 0x1000 - 1) / 0x1000);
    bit_write(1, 1, (trampoline_size + 0x1000 - 1) / 0x1000);

    total_pages = free_pages;
    bit_write(bitmap_phys / 0x1000, 1, (bitmap_len / 8 + 0x1000 - 1) / 0x1000);
    alloc((bitmap_len / 8 + 0x1000 - 1) / 0x1000);
}

void* Pmm::alloc(size_t count) {
    for (size_t i = 0; i < bitmap_len; i++) {
        if (cur_idx == bitmap_len)
            cur_idx = 0;

        if (!bitmap_is_free(cur_idx, count)) {
            cur_idx++;

            continue;
        }

        bit_write(cur_idx, 1, count);

        if (total_pages)
            free_pages -= count;

        return (void*)(cur_idx * 0x1000);
    }

    panic("OUT_OF_MEMORY");

    return nullptr;
}

void Pmm::free(void* mem, size_t count) {
    size_t idx = (size_t)mem / 0x1000;

    bit_write(idx, 0, count);

    free_pages += count;
}