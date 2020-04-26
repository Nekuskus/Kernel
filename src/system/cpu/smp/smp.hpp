#pragma once
#include <stdint.h>

namespace Cpu::Smp {
	struct [[gnu::packed]] Tss {
        uint32_t unused1;
        uint64_t rsp[4];
        uint64_t unused2;
        uint64_t ist[7];
        uint64_t unused3;
        uint16_t unused4;
        uint16_t io_bitmap_offset;
        uint8_t io_bitmap[8193];
    };

    bool wait_for_boot();
    void init();
    void boot_cpu(uint32_t lapic_id);
    void set_booted();
}  // namespace Cpu::Smp