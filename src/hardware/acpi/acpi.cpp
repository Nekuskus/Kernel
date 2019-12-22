#include <hardware/acpi/acpi.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/paging.hpp>
#include <hardware/panic.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>
#include <lib/linked_list.hpp>

Firework::Acpi::RsdpInfo rsdp_info;
auto acpi_tables = LinkedList<Firework::Acpi::SdtHeader*>();

uint8_t calculate_checksum(void* ptr, size_t size) {
    uint8_t sum = 0;

    for (size_t i = 0; i < size; i++)
        sum += ((uint8_t*)ptr)[i];

    return sum;
}

Firework::Acpi::RsdpInfo bios_detect_rsdp(uint64_t base, size_t length) {
    uint64_t address = base + virtual_physical_base;
    Firework::Acpi::RsdpInfo info{};

    Firework::Vmm::map_pages(Firework::Vmm::get_current_context(), address, base, (length + page_size - 1) / page_size, Firework::Vmm::VirtualMemoryFlags::VMM_PRESENT);

    for (size_t off = 0; off < length; off += 16) {
        Firework::Acpi::RsdpDescriptor* rsdp = (Firework::Acpi::RsdpDescriptor*)(address + off);

        if (strncmp(rsdp->signature, "RSD PTR ", 8) != 0 || calculate_checksum(rsdp, sizeof(Firework::Acpi::RsdpDescriptor)) != 0)
            continue;

        info.rsdp_address = address + off;

        for (size_t i = 0; i < 6; i++)
            info.oem_id[i] = rsdp->oem_id[i];

        info.oem_id[6] = '\0';

        if (!rsdp->revision) {
            info.version = 1;
            info.address = (uint64_t)rsdp->rsdt_address + virtual_physical_base;
            break;
        } else {
            Firework::Acpi::XsdpDescriptor* xsdp = (Firework::Acpi::XsdpDescriptor*)rsdp;

            if (calculate_checksum(xsdp, sizeof(Firework::Acpi::XsdpDescriptor)) != 0)
                continue;

            info.version = 2;
            info.address = xsdp->xsdt_address + virtual_physical_base;
            break;
        }
    }

    return info;
}

Firework::Acpi::SdtHeader* Firework::Acpi::get_table(const char* signature) {
    for (auto table : acpi_tables)
        if (strncmp(table->signature, signature, 4) == 0)
            return table;

    return nullptr;
}

void Firework::Acpi::init() {
    uint16_t* ebda_seg_ptr = (uint16_t*)(0x40E + virtual_physical_base);

    Firework::Vmm::map_pages(Firework::Vmm::get_current_context(), 0x40E + virtual_physical_base, 0x40E, (sizeof(uint16_t) + page_size - 1) / page_size, Firework::Vmm::VirtualMemoryFlags::VMM_PRESENT);

    rsdp_info = bios_detect_rsdp(*ebda_seg_ptr << 4, 0x400);

    if (!rsdp_info.version)
        rsdp_info = bios_detect_rsdp(0xE0000, 0x20000);

    if (!rsdp_info.version)
        Firework::panic("ACPI not supported");

    char text[255] = "";

    sprintf(text, "[ACPI] Detected ACPI with OEM ID %s and version %d", rsdp_info.oem_id, rsdp_info.version);
    Terminal::write_line(text, 0xFFFFFF);

    if (rsdp_info.version >= 2) {
        Firework::Acpi::XsdtHeader* xsdt = (Firework::Acpi::XsdtHeader*)rsdp_info.address;
        size_t entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;

        for (size_t i = 0; i < entries; i++) {
            if ((SdtHeader*)xsdt->tables[i] == nullptr)
                continue;

            Vmm::map_pages(Vmm::get_current_context(), xsdt->tables[i] + virtual_physical_base, xsdt->tables[i], 1, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            Firework::Acpi::SdtHeader* h = (Firework::Acpi::SdtHeader*)(xsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), xsdt->tables[i] + virtual_physical_base, xsdt->tables[i], (h->length + page_size - 1) / page_size + 2, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            if (calculate_checksum(h, h->length) == 0) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with address %x and signature %c%c%c%c", (uint64_t)h, h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Terminal::write_line(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    } else {
        Firework::Acpi::RsdtHeader* rsdt = (Firework::Acpi::RsdtHeader*)rsdp_info.address;
        size_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

        for (size_t i = 0; i < entries; i++) {
            if ((SdtHeader*)(uint64_t)rsdt->tables[i] == nullptr)
                continue;

            Vmm::map_pages(Vmm::get_current_context(), (uint64_t)rsdt->tables[i] + virtual_physical_base, (uint64_t)rsdt->tables[i], 1, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            Firework::Acpi::SdtHeader* h = (Firework::Acpi::SdtHeader*)((uint64_t)rsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), (uint64_t)rsdt->tables[i] + virtual_physical_base, (uint64_t)rsdt->tables[i], (h->length + page_size - 1) / page_size + 2, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            if (calculate_checksum(h, h->length) == 0) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with address %x and signature %c%c%c%c", (uint64_t)h, h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Terminal::write_line(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    }
}