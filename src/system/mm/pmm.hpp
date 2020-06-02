#pragma once
#include <stddef.h>
#include <stdint.h>

#include <stivale.hpp>

namespace Pmm {
    void init(Stivale::StivaleMMapEntry* mmap, size_t mmap_len);
    void* alloc(size_t count);
    void free(void* mem, size_t count);
}  // namespace Pmm