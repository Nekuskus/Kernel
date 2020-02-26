#pragma once
#include <stdint.h>

#include <lib/linked_list.hpp>

namespace Pci {
    uint32_t read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size);
    void write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size);

    class Device {
    public:
        uint8_t bus, slot, function;

        Device(uint8_t bus, uint8_t slot, uint8_t function)
            : bus(bus), slot(slot), function(function) {
        }

        uint16_t vendor_id() {
            return read(bus, slot, function, 0, 2);
        }

        uint16_t device_id() {
            return read(bus, slot, function, 2, 2);
        }

        uint16_t cmd() {
            return read(bus, slot, function, 4, 2);
        }

        void cmd(uint16_t value) {
            write(bus, slot, function, 4, value, 2);
        }

        uint16_t status() {
            return read(bus, slot, function, 6, 2);
        }

        void status(uint16_t value) {
            write(bus, slot, function, 6, value, 2);
        }

        uint8_t revision_id() {
            return read(bus, slot, function, 8, 1);
        }

        uint8_t programming_interface() {
            return read(bus, slot, function, 9, 1);
        }

        uint8_t sub_class_code() {
            return read(bus, slot, function, 10, 1);
        }

        uint8_t base_class_code() {
            return read(bus, slot, function, 11, 1);
        }

        uint8_t cache_line_size() {
            return read(bus, slot, function, 12, 1);
        }

        uint8_t master_latency_timer() {
            return read(bus, slot, function, 13, 1);
        }

        uint8_t header_type() {
            return read(bus, slot, function, 14, 1);
        }

        uint8_t bist() {
            return read(bus, slot, function, 15, 1);
        }

        void bist(uint8_t value) {
            write(bus, slot, function, 15, value, 1);
        }

        uint8_t capabilities_pointer() {
            return read(bus, slot, function, 52, 1);
        }

        uint8_t interrupt_line() {
            return read(bus, slot, function, 60, 1);
        }

        uint8_t interrupt_pin() {
            return read(bus, slot, function, 61, 1);
        }
    };

    LinkedList<Pci::Device> get_devices(uint8_t base_class_code, uint8_t sub_class_code, uint8_t programming_interface);
    void init();
}  // namespace Pci