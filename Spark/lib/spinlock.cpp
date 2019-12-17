#include <hardware/cpu/cpu.hpp>
#include <lib/spinlock.hpp>

void Spark::Spinlock::lock() {
    Cpu::atomic_set(&locked);
}

void Spark::Spinlock::release() {
    Cpu::atomic_unset(&locked);
}