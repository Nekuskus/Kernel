#pragma once
#include <stdint.h>

namespace Spark::Cpu::Smp {
    bool wait_for_boot();
    void init();
    void boot_cpu(uint32_t lapic_id);
    void set_booted();
}  // namespace Spark::Cpu::Smp