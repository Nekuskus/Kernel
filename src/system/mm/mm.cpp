#include "mm.hpp"

#include <lib/lib.hpp>
#include <lib/spinlock.hpp>

#include "pmm.hpp"
#include "vmm.hpp"

static Spinlock mm_lock{};
static uintptr_t top = heap_base;

extern "C" void* malloc(size_t bytes) {
    mm_lock.lock();

    bytes = (bytes + 7) / 8 * 8 + 16;
    size_t pages = (bytes + page_size - 1) / page_size + 1;
    void* out = (void*)top;

    for (size_t i = 0; i < pages; i++) {
        void* p = Pmm::alloc(1);

        if (!p) {
            mm_lock.release();

            return nullptr;
        }

        Vmm::map_pages(Vmm::get_current_context(), (void*)top, p, 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

        top += page_size;
    }

    top += page_size;
    out = (void*)((uintptr_t)out + (pages * page_size - bytes));
    ((uint64_t*)out)[0] = bytes - 16;
    ((uint64_t*)out)[1] = pages;

    mm_lock.release();

    return (void*)((uintptr_t)out + 16);
}

extern "C" void* calloc(size_t bytes, size_t elem) {
    void* out = malloc(bytes * elem);

    memset(out, 0, bytes * elem);

    return out;
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

extern "C" void free(void* ptr) {
    mm_lock.lock();

    size_t size = *(uint64_t*)((uintptr_t)ptr - 16), req_pages = *(uint64_t*)((uintptr_t)ptr - 8);
    uintptr_t start = (uintptr_t)ptr & ~(page_size - 1);

    size_t pages = (size + page_size - 1) / page_size + 1;

    if (req_pages != pages)
        return;

    for (size_t i = 0; i < pages; i++) {
        uint64_t curr = (uintptr_t)start + i * page_size;
        uintptr_t p = Vmm::get_entry(Vmm::get_current_context(), (void*)curr);

        Vmm::unmap_pages(Vmm::get_current_context(), (void*)curr, 1);
        Pmm::free((void*)p, 1);
    }

    mm_lock.release();
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