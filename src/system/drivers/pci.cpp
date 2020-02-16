#include "pci.hpp"

#include <lib/lib.hpp>
#include <system/acpi/acpi.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>

#include "port.hpp"

static auto mcfg_entries = LinkedList<Acpi::McfgEntry>();

uint32_t iomap_read([[maybe_unused]] uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t access_size) {
    Port::outd(0xCF8, ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)function << 8) | (offset & 0xFC) | (1u << 31));

    switch (access_size) {
        case 1:
            return Port::inb(0xCFC + (offset % 4));

        case 2:
            return Port::inw(0xCFC + (offset % 4));

        case 4:
            return Port::ind(0xCFC + (offset % 4));

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
    Port::outd(0xCF8, ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)function << 8) | (offset & 0xFC) | (1u << 31));

    switch (access_size) {
        case 1:
            Port::outb(0xCFC + (offset % 4), (uint8_t)value);

        case 2:
            Port::outw(0xCFC + (offset % 4), (uint16_t)value);

        case 4:
            Port::outd(0xCFC + (offset % 4), (uint32_t)value);

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

        char debug1[512] = "";

        sprintf(debug1, "[PCI] Accessing from %x\n", addr_virtual);
        Debug::print(debug1);

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

const char* Pci::class_code_to_str(uint8_t class_code) {
    switch (class_code) {
        case 0:
            return "Unclassified";

        case 1:
            return "Mass Storage Controller";

        case 2:
            return "Network Controller";

        case 3:
            return "Display Controller";

        case 4:
            return "Multimedia controller";

        case 5:
            return "Memory Controller";

        case 6:
            return "Bridge Device";

        case 8:
            return "Base System Peripheral";

        case 9:
            return "Input Device Controller";

        case 10:
            return "Docking Station";

        case 12:
            return "Serial Bus Controller";

        case 13:
            return "Wireless Controller";

        case 255:
            return "Unassigned Class (Vendor specific)";

        default:
            return "Unknown";
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

LinkedList<Pci::Device*> Pci::get_devices(uint8_t base_class_code, uint8_t sub_class_code, uint8_t programming_interface) {
    LinkedList<Device*> result{};

    if (mcfg_entries.length()) {
        for (auto& entry : mcfg_entries) {
            for (size_t y = entry.start_bus_number; y < entry.end_bus_number; y++) {
                for (size_t x = 0; x < 32; x++) {
                    for (uint8_t fun = 0; fun < 8; fun++) {
                        uint64_t addr = (entry.ecm_base + (((y - entry.start_bus_number) << 20) | (x << 15) | (fun << 12))) | 0;
                        uint64_t addr_virtual = addr + virtual_physical_base;

                        Vmm::map_pages(Vmm::get_ctx_kernel(), (void*)addr_virtual, (void*)addr, 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

                        Device* dev = (Device*)addr_virtual;

                        if (dev->vendor_id != 0xFFFF && dev->base_class_code == base_class_code && dev->sub_class_code == sub_class_code && dev->programming_interface == programming_interface)
                            result.push_back(dev);
                    }
                }
            }
        }
    }

    return result;
}

void Pci::init() {
    Acpi::McfgHeader* mcfg = (Acpi::McfgHeader*)Acpi::get_table("MCFG");

    if (mcfg != nullptr) {
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