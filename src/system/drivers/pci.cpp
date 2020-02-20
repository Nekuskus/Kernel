#include "pci.hpp"

#include <lib/lib.hpp>
#include <system/acpi/acpi.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>

#include "port.hpp"

static auto mcfg_entries = LinkedList<Acpi::McfgEntry>();

uint32_t iomap_read([[maybe_unused]] uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size) {
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

void iomap_write([[maybe_unused]] uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size) {
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

uint32_t (*internal_read)(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) = iomap_read;
void (*internal_write)(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t) = iomap_write;

uint32_t mmap_read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size) {
    for (const auto& entry : mcfg_entries) {
        if (entry.segment != segment && bus <= entry.start_bus_number && bus >= entry.end_bus_number)
            continue;

        uint64_t addr = (entry.ecm_base + (((bus - entry.start_bus_number) << 20) | (slot << 15) | (function << 12))) | offset;
        uint64_t addr_virtual = addr + virtual_physical_base;

        Vmm::map_pages(Vmm::get_ctx_kernel(), (void*)addr_virtual, (void*)addr, 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

        switch (access_size) {
            case 1:
                return *(uint8_t*)addr_virtual;

            case 2:
                return *(uint16_t*)addr_virtual;

            case 4:
                return *(uint32_t*)addr_virtual;


            default: {
                char debug[512] = "";

                sprintf(debug, "[PCI] Unknown access size %d\n", access_size);
                Debug::print(debug);

                return 0;
            }
        }
    }

    return 0;
}

void mmap_write(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size) {
    for (const auto& entry : mcfg_entries) {
        if (entry.segment != segment && bus <= entry.start_bus_number && bus >= entry.end_bus_number)
            continue;

        uint64_t addr = (entry.ecm_base + (((bus - entry.start_bus_number) << 20) | (slot << 25) | (function << 12))) | offset;
        uint64_t addr_virtual = addr + virtual_physical_base;

        Vmm::map_pages(Vmm::get_ctx_kernel(), (void*)addr_virtual, (void*)addr, 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

        switch (access_size) {
            case 1: {
                *(uint8_t*)addr_virtual = (uint8_t)value;

                break;
            }

            case 2: {
                *(uint16_t*)addr_virtual = (uint16_t)value;

                break;
            }

            case 4: {
                *(uint32_t*)addr_virtual = (uint32_t)value;

                break;
            }


            default: {
                char debug[512] = "";

                sprintf(debug, "[PCI] Unknown access size %d\n", access_size);
                Debug::print(debug);

                break;
            }
        }
    }
}

uint32_t Pci::read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size) {
    return internal_read(0, bus, slot, function, offset, access_size);
}

void Pci::write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size) {
    internal_write(0, bus, slot, function, offset, value, access_size);
}

uint32_t Pci::read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size) {
    return internal_read(segment, bus, slot, function, offset, access_size);
}

void Pci::write(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value, uint8_t access_size) {
    internal_write(segment, bus, slot, function, offset, value, access_size);
}

LinkedList<Pci::Device> Pci::get_devices(uint8_t base_class_code, uint8_t sub_class_code, uint8_t programming_interface) {
    LinkedList<Device> result;

    for (size_t bus = 0; bus < 256; bus++)
        for (size_t slot = 0; slot < 32; slot++)
            for (uint8_t fun = 0; fun < 8; fun++) {
                Device dev = Device(0, bus, slot, fun);

                if (dev.vendor_id() != 0xFFFF && dev.base_class_code() == base_class_code && dev.sub_class_code() == sub_class_code && dev.programming_interface() == programming_interface)
                    result.push_back(dev);
            }

    return result;
}

void Pci::init() {
    Acpi::McfgHeader* mcfg = (Acpi::McfgHeader*)Acpi::get_table("MCFG");

    if (mcfg) {
        size_t entry_len = (mcfg->header.length - (sizeof(Acpi::SdtHeader) + sizeof(uint64_t))) / sizeof(Acpi::McfgEntry);

        for (uint64_t i = 0; i < entry_len; i++) {
            Acpi::McfgEntry& entry = mcfg->entries[i];

            mcfg_entries.push_back(entry);

            char text[255] = "";

            sprintf(text, "[DEVMGR] MCFG entry #%d %p: %d:%d:0:0 - %d:%d:0:0\n", i, entry.ecm_base, entry.segment, entry.start_bus_number, entry.segment, entry.end_bus_number);
            Debug::print(text);
        }

        internal_read = mmap_read;
        internal_write = mmap_write;
    } else
        Debug::print("[DEVMGR] No PCI(E) Memory-Mapped support!\n");
}