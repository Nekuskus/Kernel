#pragma once
#include <stddef.h>
#include <stdint.h>

#include <system/acpi/acpi.hpp>

namespace Hpet {
    struct [[gnu::packed]] HpetTable {
        Acpi::SdtHeader header;
        uint8_t hardware_rev_id;
        uint8_t comparator_count : 5;
        uint8_t counter_size : 1;
        uint8_t reserved : 1;
        uint8_t legacy_replacement : 1;
        uint16_t pci_vendor_id;
        uint8_t address_space_id;
        uint8_t register_bit_width;
        uint8_t register_bit_offset;
        uint8_t reserved2;
        uint64_t address;
        uint8_t hpet_number;
        uint16_t minimum_tick;
        uint8_t page_protection;
    };

    struct HpetTimer {
        uint64_t config_and_capabilities;
        uint64_t comparator_value;
        uint64_t fsb_interupt_router;
        uint64_t unused;
    };

    struct Hpet {
        uint64_t general_capabilities;
        uint64_t unused;
        uint64_t general_configuration;
        uint64_t unused2;
        uint64_t general_int_status;
        uint64_t unused3;
        uint64_t unused4[24];
        uint64_t main_counter_value;
        uint64_t unused5;
        HpetTimer timers[];
    };

    void init();
}  // namespace Hpet

namespace Time {
    void ksleep(size_t ms);
}  // namespace Time