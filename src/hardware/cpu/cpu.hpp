#pragma once
#include <stdint.h>

namespace Spark::Cpu {
    bool check_msr(uint32_t flag);
    void halt_forever();
    void atomic_set(volatile int* var);
    void atomic_unset(volatile int* var);
};  // namespace Spark::Cpu