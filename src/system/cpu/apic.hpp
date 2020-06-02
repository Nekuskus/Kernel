#pragma once
#include <stdint.h>

inline constexpr uint32_t lapic_icr0 = 0x300;
inline constexpr uint32_t lapic_icr1 = 0x310;

namespace Cpu::Apic {
    namespace LocalApic {
        enum class TimerRegisters : uint32_t {
            LVT_TIMER = 0x320,
            INITIAL_COUNT = 0x380,
            CURRENT_COUNT = 0x390,
            DIVIDE_CONFIG = 0x3E0,
        };

        uint32_t read(uint32_t reg);
        void write(uint32_t reg, uint32_t data);
        void send_eoi();
        void send_ipi(uint32_t lapic, uint32_t vector);
    }  // namespace LocalApic

    namespace IoApic {
    }  // namespace IoApic
    void init();
}  // namespace Cpu::Apic