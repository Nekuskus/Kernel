#pragma once
#include <stdint.h>

#include <system/acpi/madt.hpp>

namespace Cpu::Apic {
    namespace LocalApic {
        constexpr uint64_t icr_low = 0x300;
        constexpr uint64_t icr_high = 0x310;
        constexpr uint64_t lapic_tpr = 0x80;

        enum IcrFlags {
            TM_LEVEL = 0x8000,
            LEVELASSERT = 0x4000,
            PENDING = 0x1000,
            DM_INIT = 0x500,
            DM_SIPI = 0x600,
        };

        enum TimerRegisters {
            TIMER_DIV = 0x3E0,
            TIMER_INITCNT = 0x380,
            TIMER_CURRCNT = 0x390,
            TIMER_LVT = 0x320,
        };

        uint32_t read(uint32_t reg);
        void write(uint32_t reg, uint32_t data);
        void send_eoi();
        void send_ipi(uint32_t target, uint32_t flags);
        void init();
    }  // namespace LocalApic

    namespace IoApic {
        constexpr uint32_t ioapic_ver = 1;

        enum DeliveryMode {
            FIXED = 0,
            LOWEST_PRIORITY = 1,
            SMI = 2,
            NMI = 4,
            INIT = 5,
            EXTINT = 7,
        };

        enum DestinationMode {
            PHYSICAL,
            LOGICAL,
        };

        uint64_t read_entry(uint8_t gsi);
        uint64_t read_entry(Madt::IoApic *ioapic, uint8_t gsi);
        void set_entry(uint8_t gsi, uint64_t data);
        void set_entry(Madt::IoApic *ioapic, uint8_t gsi, uint64_t data);
        void set_entry(uint8_t gsi, uint8_t vector, DeliveryMode delivery_mode, DestinationMode destination_mode, uint16_t flags, uint32_t destination);
        void set_entry(Madt::IoApic *ioapic, uint8_t gsi, uint8_t vector, DeliveryMode delivery_mode, DestinationMode destination_mode, uint16_t flags, uint32_t destination);
        uint32_t read(uint32_t ioapic_base, uint32_t reg);
        void write(uint32_t ioapic_base, uint32_t reg, uint32_t data);
        void mask_gsi(uint32_t gsi);
        void unmask_gsi(uint32_t gsi);
        void mask_irq(uint32_t irq);
        void unmask_irq(uint32_t irq);
        void init();
    }  // namespace IoApic
}  // namespace Cpu::Apic