#include <hardware/acpi/acpi.hpp>
#include <hardware/devices/pci.hpp>
#include <hardware/mm/vmm.hpp>
#include <hardware/port.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>
#include <lib/linked_list.hpp>

auto mcfg_entries = LinkedList<Spark::Acpi::McfgEntry>();

using read_function = uint32_t (*)(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t);
using write_function = void (*)(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t);

uint32_t iomap_read([[maybe_unused]] uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    Spark::Port::outd(0xCF8, ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)function << 8) | (offset & 0xFC) | (1u << 31));

    return Spark::Port::ind(0xCFC + (offset % 4));
}

void iomap_write([[maybe_unused]] uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value) {
    Spark::Port::outd(0xCF8, ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)function << 8) | (offset & 0xFC) | (1u << 31));
    Spark::Port::outd(0xCFC + (offset % 4), value);
}

read_function internal_read = iomap_read;
write_function internal_write = iomap_write;

uint32_t mmap_read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    for (const auto& entry : mcfg_entries) {
        if (entry.segment != segment && bus <= entry.start_bus_number && bus >= entry.end_bus_number)
            continue;

        uint64_t addr = (entry.ecm_base + (((bus - entry.start_bus_number) << 20) | (slot << 25) | (function << 12))) | offset;
        uint64_t addr_virtual = addr + virtual_physical_base;

        Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), (void*)addr_virtual, (void*)addr, 1, Spark::Vmm::VirtualMemoryFlags::VMM_PRESENT | Spark::Vmm::VirtualMemoryFlags::VMM_WRITE);
        return *(uint32_t*)addr_virtual;
    }

    return 0;
}

void mmap_write(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value) {
    for (const auto& entry : mcfg_entries) {
        if (entry.segment != segment && bus <= entry.start_bus_number && bus >= entry.end_bus_number)
            continue;

        uint64_t addr = (entry.ecm_base + (((bus - entry.start_bus_number) << 20) | (slot << 25) | (function << 12))) | offset;
        uint64_t addr_virtual = addr + virtual_physical_base;

        Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), (void*)addr_virtual, (void*)addr, 1, Spark::Vmm::VirtualMemoryFlags::VMM_PRESENT | Spark::Vmm::VirtualMemoryFlags::VMM_WRITE);
        *(uint32_t*)addr_virtual = value;
    }
}

const char* Spark::Pci::class_code_to_str(uint8_t class_code) {
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
    }

    return "Unknown";
}

uint32_t Spark::Pci::read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return internal_read(0, bus, slot, function, offset);
}

uint32_t Spark::Pci::read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return internal_read(segment, bus, slot, function, offset);
}

void Spark::Pci::write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value) {
    internal_write(0, bus, slot, function, offset, value);
}

void Spark::Pci::write(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value) {
    internal_write(segment, bus, slot, function, offset, value);
}

void Spark::Pci::init() {
    Acpi::McfgHeader* mcfg = (Acpi::McfgHeader*)Acpi::get_table("MCFG");

    if (mcfg != nullptr) {
        size_t entry_len = (mcfg->header.length - (sizeof(Acpi::SdtHeader) + sizeof(uint64_t))) / sizeof(Acpi::McfgEntry);
        for (uint64_t i = 0; i < entry_len; i++) {
            Acpi::McfgEntry& entry = mcfg->entries[i];
            mcfg_entries.push_back(entry);
            char text[255] = "";
            sprintf(text, "[DEVMGR] MCFG entry #%d %p: %d:%d:0:0 - %d:%d:0:0", i, entry.ecm_base, entry.segment, entry.start_bus_number, entry.segment, entry.end_bus_number);
            Terminal::write_line(text, 0xFFFFFF);
        }

        internal_read = mmap_read;
        internal_write = mmap_write;
    } else
        Terminal::write_line("[DEVMGR] No PCI Express support", 0xFFFFFF);
}