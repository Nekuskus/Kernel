#pragma once
#include <stdint.h>

namespace Firework::FireworkKernel::Pci {
    enum HeaderType {
        STANDARD = 0,
        PCI_TO_PCI_BRIDGE = 1,
        CARDBUS_BRIDGE = 2,
    };

    struct [[gnu::packed]] Device {
        uint16_t device_id;
        uint16_t vendor_id;
        uint8_t class_code;
        uint8_t subclass;
        uint8_t prog_if;
        uint8_t revision_id;
        uint8_t bist;
        uint8_t header_type;
        uint8_t latency_timer;
        uint8_t cache_line_size;
    };

    struct [[gnu::packed]] NonBridge {
        Device device;
        uint32_t bar0;
        uint32_t bar1;
        uint32_t bar2;
        uint32_t bar3;
        uint32_t bar4;
        uint32_t bar5;
        uint32_t cardbus_cis_pointer;
        uint16_t subsystem_id;
        uint16_t subsystem_vendor_id;
        uint32_t expansion_rom_base;
        uint16_t reserved;
        uint8_t reserved2;
        uint8_t capabilities_pointer;
        uint32_t reserved3;
        uint8_t max_latency;
        uint8_t min_grant;
        uint8_t interrupt_pin;
        uint8_t interrupt_line;
    };

    struct [[gnu::packed]] PciToPciBridge {
        Device device;
        uint32_t bar0;
        uint32_t bar1;
        uint8_t secondary_latency_timer;
        uint8_t subordinate_bus_num;
        uint8_t secondary_bus_num;
        uint8_t primary_bus_num;
        uint16_t secondary_status;
        uint8_t io_limit;
        uint8_t io_base;
        uint16_t memory_limit;
        uint16_t memory_base;
        uint16_t prefetchable_mem_limit;
        uint16_t prefetchable_mem_base;
        uint32_t io_base_upper_32bits;
        uint32_t io_limit_upper_32bits;
        uint16_t io_limit_upper_16bits;
        uint16_t io_base_upper_16bits;
        uint16_t reserved;
        uint8_t reserved2;
        uint8_t capability_pointer;
        uint32_t expansion_rom_base;
        uint16_t bridge_control;
        uint8_t interrupt_pin;
        uint8_t interrupt_line;
    };

    struct [[gnu::packed]] PciToCardbusBridge {
        Device device;
        uint32_t cardbus_socket_base;
        uint16_t secondary_status;
        uint8_t reserved;
        uint8_t capabilities_list_offset;
        uint8_t cardbus_latency_timer;
        uint8_t subordinate_bus_num;
        uint8_t cardbus_bus_num;
        uint8_t pci_bus_num;
        uint32_t memory_base0;
        uint32_t memory_limit0;
        uint32_t memory_base1;
        uint32_t memory_limit1;
        uint32_t io_base0;
        uint32_t io_limit0;
        uint32_t io_base1;
        uint32_t io_limit1;
        uint16_t bridge_control;
        uint8_t interrupt_pin;
        uint8_t interrupt_line;
        uint16_t subsystem_id;
        uint16_t subsystem_vendor_id;
        uint32_t pc_card_legacy_mode_base;
    };

    uint32_t read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
    void write(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value);
    uint32_t read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
    void write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value);
    const char* class_code_to_str(uint8_t class_code);
    void scan_all_buses();
    void init();
}  // namespace Firework::FireworkKernel::Pci