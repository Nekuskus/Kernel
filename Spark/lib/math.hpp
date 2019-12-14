#pragma once
#include <stddef.h>

namespace Spark::Math {
    size_t round_up(size_t n, size_t s);
    size_t round_down(size_t n, size_t s);
    size_t overlaps(size_t a, size_t as, size_t b, size_t bs);
    size_t max(size_t x, size_t y);
};  // namespace Spark::Math