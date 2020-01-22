#pragma once
#include <stdint.h>

namespace Cpu {
    uint32_t get_current_cpu();
    bool check_msr(uint32_t flag);
    void halt_forever();
    void atomic_set(volatile int* var);
    void atomic_unset(volatile int* var);
}  // namespace Cpu