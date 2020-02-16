#pragma once
#include <stdint.h>

#include <lib/linked_list.hpp>

namespace Pci {
    enum HeaderType {
        STANDARD = 0,
        PCI_TO_PCI_BRIDGE = 1,
        CARDBUS_BRIDGE = 2,
    };

    struct [[gnu::packed]] Device {
        uint16_t vendor_id;
        uint16_t device_id;
        uint16_t cmd;
        uint16_t status;
        uint8_t revision_id;
        uint8_t programming_interface;
        uint8_t sub_class_code;
        uint8_t base_class_code;
        uint8_t cache_line_size;
        uint8_t master_latency_timer;
        uint8_t header_type;
        uint8_t bist;
    };

    uint32_t read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size);
    void write(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size);
    uint32_t read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size);
    void write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size);
    const char* class_code_to_str(uint8_t class_code);
    LinkedList<Device*> get_devices(uint8_t base_class_code, uint8_t sub_class_code, uint8_t programming_interface);
    void init();
}  // namespace Pci