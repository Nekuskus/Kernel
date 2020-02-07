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

void Cpu::atomic_set(volatile int* var) {
    asm volatile(
        "1:\n\t"
        "lock bts $0, %0\n\t"
        "jnc 2f\n\t"
        "pause\n\t"
        "jmp 1b\n\t"
        "2:\n\t"
        : "+m"(*var)
        :
        : "memory", "cc");
}

void Cpu::atomic_unset(volatile int* var) {
    asm volatile("lock btr $0, %0\n\t"
                 : "+m"(*var)
                 :
                 : "memory", "cc");
}