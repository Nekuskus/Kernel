#pragma once
#include <stdint.h>

namespace Spark::Cpu::Smp {
    struct CpuEntry {
        uint32_t lapic_id;
        bool bsp;
    };

    uint64_t get_len();
    bool wait_for_boot();
    void init();
    void boot_cpu(CpuEntry cpu);
    void set_booted();
}  // namespace Spark::Cpu::Smp