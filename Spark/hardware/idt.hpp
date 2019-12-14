#pragma once
#include <stdint.h>

namespace Spark::Idt {
    struct [[gnu::packed]] Entry {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t ist;
        uint8_t attrib;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t zero;
    };

    struct [[gnu::packed]] Pointer {
        uint16_t limit;
        uint64_t base;
    };

    void init();
    void set_gate(uint8_t vec, uintptr_t function, uint16_t selector, uint8_t flags);
};  // namespace Spark::Idt