#pragma once
#include <stdint.h>

namespace Cpu::Smp {
    bool wait_for_boot();
    void init();
    void boot_cpu(uint64_t kernel_pml4, uint32_t lapic_id);
    void set_booted();
}  // namespace Cpu::Smp