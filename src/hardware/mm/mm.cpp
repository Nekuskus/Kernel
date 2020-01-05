#include <hardware/mm/mm.hpp>
#include <hardware/mm/pmm.hpp>
#include <hardware/mm/vmm.hpp>
#include <lib/lib.hpp>
#include <lib/spinlock.hpp>

Firework::FireworkKernel::Spinlock mm_lock{};
uintptr_t top = memory_base;

extern "C" void* malloc(size_t bytes) {
    mm_lock.lock();

    bytes = (((bytes + 7) / 8) * 8) + 16;
    size_t pages = (bytes + page_size - 1) / page_size + 1;
    void *out = (void*)top, *p = Firework::FireworkKernel::Pmm::alloc(pages);

    if (!p) {
        mm_lock.release();

        return nullptr;
    }

    Firework::FireworkKernel::Vmm::map_pages(Firework::FireworkKernel::Vmm::get_current_context(), top, (uint64_t)p, pages, Firework::FireworkKernel::Vmm::VirtualMemoryFlags::VMM_PRESENT | Firework::FireworkKernel::Vmm::VirtualMemoryFlags::VMM_WRITE);

    top += page_size * pages;
    out = (void*)((uintptr_t)out + (pages * page_size - bytes));
    ((uint64_t*)out)[0] = bytes - 16;
    ((uint64_t*)out)[1] = pages;

    mm_lock.release();

    return (void*)((uintptr_t)out + 16);
}

extern "C" void* calloc(size_t blocks) {
    void* out = malloc(blocks);

    memset(out, 0, blocks);

    return out;
}

extern "C" void* realloc(void* old, size_t s) {
    void* newm = malloc(s);

    if (old) {
        mm_lock.lock();

        uint64_t size = (uint64_t)old - 16;

        mm_lock.release();
        memcpy(newm, old, size);
        free(old);
    }

    return newm;
}

extern "C" void free(void* memory) {
    mm_lock.lock();

    size_t size = ((uint64_t)((uintptr_t)memory - 16)) + 16, pages = (size + page_size - 1) / page_size + 1;
    uint64_t start = (uintptr_t)memory & (~(page_size - 1));

    if (pages != (uint64_t)memory - 8)
        return;

    uintptr_t p = Firework::FireworkKernel::Vmm::get_entry(Firework::FireworkKernel::Vmm::get_current_context(), start);

    Firework::FireworkKernel::Vmm::unmap_pages(Firework::FireworkKernel::Vmm::get_current_context(), start, pages);
    Firework::FireworkKernel::Pmm::free(p, pages);
    mm_lock.release();
}

extern "C" void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    unsigned char fill = (unsigned char)c;

    while (n--)
        *p++ = fill;

    return s;
}

extern "C" void* memcpy(void* dest, const void* src, size_t len) {
    char *d = (char*)dest, *s = (char*)src;

    while (len--)
        *d++ = *s++;

    return dest;
}