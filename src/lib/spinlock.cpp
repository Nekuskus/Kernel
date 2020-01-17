#include "spinlock.hpp"
#include <hardware/cpu/cpu.hpp>

void Firework::FireworkKernel::Spinlock::lock() {
    Cpu::atomic_set(&locked);
}

void Firework::FireworkKernel::Spinlock::release() {
    Cpu::atomic_unset(&locked);
}