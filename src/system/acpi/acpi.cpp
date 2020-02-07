#include "acpi.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/panic.hpp>
#include <system/terminal.hpp>

static Acpi::RsdpInfo rsdp_info{};
static auto acpi_tables = LinkedList<Acpi::SdtHeader*>();

uint8_t calculate_checksum(void* ptr, size_t size) {
    uint8_t sum = 0;

    for (size_t i = 0; i < size; i++)
        sum += ((uint8_t*)ptr)[i];

    return sum;
}

Acpi::RsdpInfo bios_detect_rsdp(uint64_t base, size_t length) {
    uint64_t address = base + virtual_physical_base;
    Acpi::RsdpInfo info{};

    Vmm::map_pages(Vmm::get_current_context(), (void*)address, (void*)base, (length + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT);

    for (size_t off = 0; off < length; off += 16) {
        Acpi::RsdpDescriptor* rsdp = (Acpi::RsdpDescriptor*)(address + off);

        if (strncmp(rsdp->signature, "RSD PTR ", 8) || calculate_checksum(rsdp, sizeof(Acpi::RsdpDescriptor)))
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
            Acpi::XsdpDescriptor* xsdp = (Acpi::XsdpDescriptor*)rsdp;

            if (calculate_checksum(xsdp, sizeof(Acpi::XsdpDescriptor)))
                continue;

            info.version = 2;
            info.address = xsdp->xsdt_address + virtual_physical_base;
            break;
        }
    }

    return info;
}

Acpi::SdtHeader* Acpi::get_table(const char* signature) {
    for (auto& table : acpi_tables)
        if (!strncmp(table->signature, signature, 4))
            return table;

    return nullptr;
}

void Acpi::init() {
    uint16_t* ebda_seg_ptr = (uint16_t*)(0x40E + virtual_physical_base);

    Vmm::map_pages(Vmm::get_current_context(), (void*)(0x40E + virtual_physical_base), (void*)0x40E, (sizeof(uint16_t) + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT);

    rsdp_info = bios_detect_rsdp(*ebda_seg_ptr << 4, 0x400);

    if (!rsdp_info.version)
        rsdp_info = bios_detect_rsdp(0xE0000, 0x20000);

    if (!rsdp_info.version)
        panic("ACPI not supported");

    char text[255] = "";

    sprintf(text, "[ACPI] Detected ACPI with OEM ID '%s' and version %d", rsdp_info.oem_id, rsdp_info.version);
    Terminal::write_line(text, 0xFFFFFF);

    if (rsdp_info.version >= 2) {
        XsdtHeader* xsdt = (XsdtHeader*)rsdp_info.address;
        size_t entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;

        for (size_t i = 0; i < entries; i++) {
            Vmm::map_pages(Vmm::get_current_context(), (void*)(xsdt->tables[i] + virtual_physical_base), (void*)xsdt->tables[i], 1, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            SdtHeader* h = (SdtHeader*)(xsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), (void*)(xsdt->tables[i] + virtual_physical_base), (void*)xsdt->tables[i], (h->length + page_size - 1) / page_size + 2, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            if (!calculate_checksum(h, h->length)) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with address %x and signature %c%c%c%c", (uint64_t)h - virtual_physical_base, h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Terminal::write_line(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    } else {
        RsdtHeader* rsdt = (RsdtHeader*)rsdp_info.address;
        size_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

        for (size_t i = 0; i < entries; i++) {
            Vmm::map_pages(Vmm::get_current_context(), (void*)((uint64_t)rsdt->tables[i] + virtual_physical_base), (void*)(uint64_t)rsdt->tables[i], 1, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            SdtHeader* h = (SdtHeader*)((uint64_t)rsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), (void*)((uint64_t)rsdt->tables[i] + virtual_physical_base), (void*)(uint64_t)rsdt->tables[i], (h->length + page_size - 1) / page_size + 2, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            if (!calculate_checksum(h, h->length)) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with address %x and signature %c%c%c%c", (uint64_t)h - virtual_physical_base, h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Terminal::write_line(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    }
}