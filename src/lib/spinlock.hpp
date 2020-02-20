#pragma once
#include <stdatomic.h>

class Spinlock {
    atomic_bool locked;

public:
    Spinlock();
    ~Spinlock();
    void lock();
    void release();
};