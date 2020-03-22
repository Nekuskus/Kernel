#include "mm.hpp"

#include <lib/lib.hpp>
#include <lib/spinlock.hpp>

#include "pmm.hpp"
#include "vmm.hpp"

static Spinlock mm_lock{};
static uint64_t top = heap_base;

extern "C" void* malloc(size_t bytes) {
    mm_lock.lock();

    bytes = (bytes + 7) / 8 * 8;  // Round up size to a multiple of 8
    uint64_t size = bytes + sizeof(HeapHeader), pages = (size + page_size - 1) / page_size + 1, out = top;
    void* p = Pmm::alloc(pages);

    if (!p) {
        mm_lock.release();

        return nullptr;
    }

    Vmm::map_pages(Vmm::get_ctx_kernel(), (void*)top, p, pages, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

    top += page_size * (pages + 1);
    out = out + (page_size * pages - size);
    HeapHeader* header = (HeapHeader*)out;
    header->size = bytes;
    header->pages = pages;

    mm_lock.release();

    return (void*)(out + sizeof(HeapHeader));
}

extern "C" void* calloc(size_t bytes, size_t elem) {
    void* out = malloc(bytes * elem);

    memset(out, 0, bytes * elem);

    return out;
}

extern "C" void free(void* ptr) {
    mm_lock.lock();

    HeapHeader* header = (HeapHeader*)((uint64_t)ptr - 16);
    uintptr_t start = (uintptr_t)ptr & ~(page_size - 1);
    size_t pages = (header->size + page_size - 1) / page_size + 1;

    if (header->pages != pages)
        return;

    Vmm::PageTable* current_ctx = Vmm::get_current_context();
    uint64_t curr = (uintptr_t)start + pages * page_size, p = Vmm::get_entry(current_ctx, (void*)curr);

    Vmm::unmap_pages(current_ctx, (void*)curr, pages);
    Pmm::free((void*)p, pages);

    mm_lock.release();
}

extern "C" void* realloc(void* old, size_t s) {
    void* newp = malloc(s);

    if (old) {
        mm_lock.lock();

        uint64_t size = *(uint64_t*)((uint64_t)old - 16);

        mm_lock.release();
        memcpy(newp, old, size);
        free(old);
    }

    return newp;
}

void* operator new(size_t size) {
    return calloc(size, 1);
}

void* operator new[](size_t size) {
    return calloc(size, 1);
}

void operator delete(void* p) {
    free(p);
}

void operator delete[](void* p) {
    free(p);
}

void operator delete(void* p, [[maybe_unused]] long unsigned int size) {
    free(p);
}

void operator delete[](void* p, [[maybe_unused]] long unsigned int size) {
    free(p);
}