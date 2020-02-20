#pragma once
#include <stdint.h>

namespace Cpu {
    uint32_t get_current_cpu();
    bool check_msr(uint32_t flag);
    void halt_forever();
}  // namespace Cpu