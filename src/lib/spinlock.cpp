#include "spinlock.hpp"

Spinlock::Spinlock() {
    release();
}

Spinlock::~Spinlock() {
    release();
}

void Spinlock::lock() {
    locked = true;
}

void Spinlock::release() {
    locked = false;
}