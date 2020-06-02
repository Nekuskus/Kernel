#pragma once
#include <stddef.h>
#include <stdint.h>

#include "e820.hpp"

namespace Pmm {
    void init(E820::E820Entry* mmap, size_t mmap_len);
    void* alloc(size_t count);
    void free(void* mem, size_t count);
}  // namespace Pmm