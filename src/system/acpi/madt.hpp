#pragma once
#include <stdint.h>

#include <lib/linked_list.hpp>

#include "acpi.hpp"

namespace Madt {
    enum InterruptControllerType {
        LAPIC = 0,
        IOAPIC = 1,
        INTERRUPT_SOURCE_OVERRIDE = 2,
        NMI_SOURCE = 3,
        LAPIC_NMI = 4,
        LAPIC_ADDRESS_OVERRIDE = 5,
        IOSAPIC = 6,
        LOCAL_SAPIC = 7,
        PLATFORM_INTERRUPT_SOURCES = 8,
        PROCESSOR_LOCAL_X2APIC = 9,
        LOCAL_X2APIC_NMI = 10,
        GICC = 11,
        GICD = 12,
        GIC_MSI_FRAME = 13,
        GICR = 14,
        ITS = 15,
    };

    struct [[gnu::packed]] InterruptController {
        uint8_t type;
        uint8_t length;
    };

    struct [[gnu::packed]] LocalApic {
        InterruptController ic;
        uint8_t uid;
        uint8_t id;
        uint32_t flags;
    };

    struct [[gnu::packed]] IoApic {
        InterruptController ic;
        uint8_t apic_id;
        uint8_t reserved;
        uint32_t ioapic_base;
        uint32_t gsi_base;
    };

    struct [[gnu::packed]] InterruptSourceOverride {
        InterruptController ic;
        uint8_t bus;
        uint8_t source;
        uint32_t gsi;
        uint16_t flags;
    };

    struct [[gnu::packed]] MadtHeader {
        Acpi::SdtHeader header;
        uint32_t apic_address;
        uint32_t flags;
        InterruptController interrupt_controllers[];
    };

    LinkedList<Madt::LocalApic *> &get_lapics();
    LinkedList<Madt::IoApic *> &get_ioapics();
    LinkedList<Madt::InterruptSourceOverride *> &get_isos();
    bool has_legacy_pic();
    void init();
}  // namespace Madt