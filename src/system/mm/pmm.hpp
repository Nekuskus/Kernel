#pragma once
#include <stddef.h>
#include <stdint.h>

#include <multiboot.hpp>

namespace Pmm {
    void init(Multiboot::MemoryMap* mmap, size_t mmap_len);
    void* alloc(size_t count, size_t alignment, uintptr_t upper);
    void* alloc(size_t count);
    void free(void* mem, size_t count);
}  // namespace Pmm