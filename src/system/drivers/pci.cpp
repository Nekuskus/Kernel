#include "pci.hpp"

#include <lib/lib.hpp>
#include <system/acpi/acpi.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>

#include "port.hpp"

uint32_t Pci::read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size) {
    Port::outd(0xCF8, (bus << 16) | (slot << 11) | (function << 8) | (offset & ~(uint32_t)3) | 0x80000000);

    switch (access_size) {
        case 1:
            return Port::inb(0xCFC + (offset & 3));

        case 2:
            return Port::inw(0xCFC + (offset & 3));

        case 4:
            return Port::ind(0xCFC + (offset & 3));

        default: {
            char debug[512] = "";

            sprintf(debug, "[PCI] Unknown access size %d\n", access_size);
            Debug::print(debug);

            return 0;
        }
    }

    return 0;
}

void Pci::write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size) {
    Port::outd(0xCF8, (bus << 16) | (slot << 11) | (function << 8) | (offset & ~(uint32_t)3) | 0x80000000);

    switch (access_size) {
        case 1:
            Port::outb(0xCFC + (offset & 3), (uint8_t)value);

        case 2:
            Port::outw(0xCFC + (offset & 3), (uint16_t)value);

        case 4:
            Port::outd(0xCFC + (offset & 3), (uint32_t)value);

        default: {
            char debug[512] = "";

            sprintf(debug, "[PCI] Unknown access size %d\n", access_size);
            Debug::print(debug);

            break;
        }
    }
}

LinkedList<Pci::Device> Pci::get_devices(uint8_t base_class_code, uint8_t sub_class_code, uint8_t programming_interface) {
    LinkedList<Device> result;

    for (size_t bus = 0; bus < 256; bus++)
        for (size_t slot = 0; slot < 32; slot++)
            for (uint8_t fun = 0; fun < 8; fun++) {
                Device dev = Device(bus, slot, fun);

                if (dev.vendor_id() != 0xFFFF && dev.base_class_code() == base_class_code && dev.sub_class_code() == sub_class_code && dev.programming_interface() == programming_interface)
                    result.push_back(dev);
            }

    return result;
}