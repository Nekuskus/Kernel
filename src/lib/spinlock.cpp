#include <hardware/cpu/cpu.hpp>
#include <lib/spinlock.hpp>

void Firework::FireworkKernel::Spinlock::lock() {
    Cpu::atomic_set(&locked);
}

void Firework::FireworkKernel::Spinlock::release() {
    Cpu::atomic_unset(&locked);
}