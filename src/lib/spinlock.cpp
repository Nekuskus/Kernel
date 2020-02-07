#include "spinlock.hpp"

#include <system/cpu/cpu.hpp>

Spinlock::Spinlock() {
    release();
}

Spinlock::~Spinlock() {
    release();
}

void Spinlock::lock() {
    Cpu::atomic_set(&locked);
}

void Spinlock::release() {
    Cpu::atomic_unset(&locked);
}