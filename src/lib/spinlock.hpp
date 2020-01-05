#pragma once

namespace Firework::FireworkKernel {
    class Spinlock {
        volatile int locked;

    public:
        void lock();
        void release();
    };
}  // namespace Firework::FireworkKernel