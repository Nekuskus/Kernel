#include <hardware/cpu/cpu.hpp>
#include <lib/spinlock.hpp>

void Firework::Spinlock::lock() {
    Cpu::atomic_set(&locked);
}

void Firework::Spinlock::release() {
    Cpu::atomic_unset(&locked);
}