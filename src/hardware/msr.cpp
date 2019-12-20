#include <hardware/msr.hpp>

uint64_t Spark::Msr::read(uint32_t msr) {
    uint64_t value_low = 0, value_high = 0;

    asm volatile("rdmsr"
                 : "=a"(value_low), "=d"(value_high)
                 : "c"(msr));

    return value_low | (value_high << 32);
}

void Spark::Msr::write(uint32_t msr, uint64_t value) {
    asm volatile("wrmsr"
                 :
                 : "a"((uint32_t)value), "d"((uint32_t)(value >> 32)), "c"(msr));
}