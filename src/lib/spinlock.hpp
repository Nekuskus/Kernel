#pragma once

class Spinlock {
    volatile int locked;

public:
    Spinlock();
    ~Spinlock();
    void lock();
    void release();
};