#include <lib/math.hpp>

size_t Spark::Math::round_up(size_t n, size_t s) {
    return n + s - 1;
}

size_t Spark::Math::round_down(size_t n, size_t s) {
    return (n / s) * s;
}

size_t Spark::Math::overlaps(size_t a, size_t as, size_t b, size_t bs) {
    return a >= b && a + as <= b + bs;
}

size_t Spark::Math::max(size_t x, size_t y) {
    return x > y ? x : y;
}