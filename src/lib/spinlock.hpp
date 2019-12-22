#pragma once

namespace Firework {
    class Spinlock {
        volatile int locked;

    public:
        void lock();
        void release();
    };
}  // namespace Firework