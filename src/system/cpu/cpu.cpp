#include "cpu.hpp"

#include <cpuid.h>

uint32_t Cpu::get_current_cpu() {
    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);

    return (b >> 24) & 0xFF;
}

bool Cpu::check_msr(uint32_t flag) {
    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);

    return d & flag;
}

void Cpu::halt_forever() {
    asm volatile(
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b"
        :
        :
        : "memory");
}