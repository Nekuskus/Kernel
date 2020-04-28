#pragma once
#include <stdint.h>
#include "cpu/cpu.hpp"

namespace Idt {
    struct [[gnu::packed]] Entry {
        uint16_t offset_low;
        uint16_t selector;
        uint16_t flags;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t reserved;
    };

    struct [[gnu::packed]] Pointer {
        uint16_t limit;
        uint64_t base;
    };

    struct InterruptHandler {
        void (*function)(const Cpu::Registers*);
        bool is_irq;
        bool should_iret;
    };

    void register_interrupt_handler(uint16_t n, void (*function)(const Cpu::Registers*), bool is_irq, bool should_iret);
    void set_irq(uint16_t n, bool is_irq);
    void init();
}  // namespace Idt