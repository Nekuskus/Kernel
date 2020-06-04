#include "mm.hpp"

#include <lib/lib.hpp>
#include <lib/spinlock.hpp>

#include "pmm.hpp"
#include "vmm.hpp"

static uint32_t lock = 0;
static uint64_t top = heap_base;

extern "C" void *malloc(size_t bytes) {
    acquire_lock(&lock);

    bytes = (bytes + 7) / 8 * 8;
    uint64_t size = bytes + sizeof(HeapHeader), pages = (size + 0x1000 - 1) / 0x1000 + 1, out = top;
    void *p = Pmm::alloc(pages);

    if (!p) {
        release_lock(&lock);

        return nullptr;
    }

    Vmm::map_pages(Vmm::get_ctx_kernel(), (void *)top, p, pages, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);

    top += 0x1000 * (pages + 1);
    out = out + (0x1000 * pages - size);

    auto header = (HeapHeader *)out;
    header->size = bytes;
    header->pages = pages;

    release_lock(&lock);

    return (void *)(out + sizeof(HeapHeader));
}

extern "C" void *calloc(size_t bytes, size_t elem) {
    auto out = malloc(bytes * elem);

    memset(out, 0, bytes * elem);

    return out;
}

extern "C" void free(void *ptr) {
    acquire_lock(&lock);

    auto header = (HeapHeader *)((uint64_t)ptr - 16);
    uint64_t start = (uint64_t)ptr & ~(0x1000 - 1);
    size_t pages = (header->size + 0x1000 - 1) / 0x1000 + 1;

    if (header->pages != pages)
        return;

    auto current_ctx = Vmm::get_current_context();
    uint64_t curr = (uint64_t)start + pages * 0x1000, p = Vmm::get_entry(current_ctx, (void *)curr);

    Vmm::unmap_pages(current_ctx, (void *)curr, pages);
    Pmm::free((void *)p, pages);

    release_lock(&lock);
}

extern "C" void *realloc(void *old, size_t s) {
    auto newp = malloc(s);

    if (old) {
        acquire_lock(&lock);

        auto size = *(uint64_t *)((uint64_t)old - 16);

        release_lock(&lock);
        memcpy(newp, old, size);
        free(old);
    }

    return newp;
}

void *operator new(size_t size) {
    return calloc(size, 1);
}

void *operator new[](size_t size) {
    return calloc(size, 1);
}

void operator delete(void *p) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

void operator delete(void *p, [[maybe_unused]] long unsigned int size) {
    free(p);
}

void operator delete[](void *p, [[maybe_unused]] long unsigned int size) {
    free(p);
}