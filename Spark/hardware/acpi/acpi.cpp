#include <hardware/acpi/acpi.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/vmm.hpp>
#include <hardware/panic.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>
#include <lib/linked_list.hpp>

Spark::Acpi::RsdpInfo rsdp_info;
auto acpi_tables = LinkedList<Spark::Acpi::SdtHeader*>();

inline uint8_t calculate_checksum(void* ptr, size_t size) {
    uint8_t sum = 0;

    for (size_t i = 0; i < size; i++)
        sum += ((uint8_t*)ptr)[i];

    return sum;
}

inline Spark::Acpi::RsdpInfo bios_detect_rsdp(uint64_t base, size_t length) {
    uint64_t address = base + virtual_physical_base;
    Spark::Acpi::RsdpInfo info{};

    Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), (void*)address, (void*)base, (length + page_size - 1) / page_size, Spark::Vmm::VirtualMemoryFlags::VMM_PRESENT);

    for (size_t off = 0; off < length; off += 16) {
        Spark::Acpi::RsdpDescriptor* rsdp = (Spark::Acpi::RsdpDescriptor*)(address + off);

        if (strncmp(rsdp->signature, "RSD PTR ", 8) != 0 || calculate_checksum(rsdp, sizeof(Spark::Acpi::RsdpDescriptor)) != 0)
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
            Spark::Acpi::XsdpDescriptor* xsdp = (Spark::Acpi::XsdpDescriptor*)rsdp;

            if (calculate_checksum(xsdp, sizeof(Spark::Acpi::XsdpDescriptor)) != 0)
                continue;

            info.version = 2;
            info.address = xsdp->xsdt_address + virtual_physical_base;
            break;
        }
    }

    return info;
}

inline Spark::Acpi::RsdpInfo bios_detect_rsdp() {
    uint16_t* ebda_seg_ptr = (uint16_t*)(0x40E + virtual_physical_base);

    Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), (void*)(0x40E + virtual_physical_base), (void*)0x40E, (sizeof(uint16_t) + page_size - 1) / page_size, Spark::Vmm::VirtualMemoryFlags::VMM_PRESENT);

    Spark::Acpi::RsdpInfo info = bios_detect_rsdp(*ebda_seg_ptr << 4, 0x400);

    if (!info.version)
        info = bios_detect_rsdp(0xE0000, 0x20000);

    if (!info.version)
        Spark::panic("ACPI not supported");

    return info;
}

Spark::Acpi::SdtHeader* Spark::Acpi::get_table(const char* signature) {
    for (auto table : acpi_tables)
        if (strncmp(table->signature, signature, 4) == 0)
            return table;

    return nullptr;
}

void Spark::Acpi::init() {
    rsdp_info = bios_detect_rsdp();
    char text[255] = "";

    sprintf(text, "[ACPI] Detected ACPI with OEM ID %s and version %d", rsdp_info.oem_id, rsdp_info.version);
    Terminal::write_line(text, 0xFFFFFF);

    if (rsdp_info.version >= 2) {
        Spark::Acpi::XsdtHeader* xsdt = (Spark::Acpi::XsdtHeader*)rsdp_info.address;
        size_t entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;

        for (size_t i = 0; i < entries; i++) {
            if ((SdtHeader*)xsdt->tables[i] == nullptr)
                continue;

            Vmm::map_pages(Vmm::get_current_context(), (void*)(xsdt->tables[i] + virtual_physical_base), (void*)xsdt->tables[i], 1, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            Spark::Acpi::SdtHeader* h = (Spark::Acpi::SdtHeader*)(xsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), (void*)(xsdt->tables[i] + virtual_physical_base), (void*)xsdt->tables[i], (h->length + page_size - 1) / page_size + 2, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            if (calculate_checksum(h, h->length) == 0) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with signature %c%c%c%c", h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Terminal::write_line(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    } else {
        Spark::Acpi::RsdtHeader* rsdt = (Spark::Acpi::RsdtHeader*)rsdp_info.address;
        size_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

        for (size_t i = 0; i < entries; i++) {
            if ((SdtHeader*)(uint64_t)rsdt->tables[i] == nullptr)
                continue;

            Vmm::map_pages(Vmm::get_current_context(), (void*)(rsdt->tables[i] + virtual_physical_base), (void*)(uint64_t)rsdt->tables[i], 1, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            Spark::Acpi::SdtHeader* h = (Spark::Acpi::SdtHeader*)((uint64_t)rsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), (void*)(rsdt->tables[i] + virtual_physical_base), (void*)(uint64_t)rsdt->tables[i], (h->length + page_size - 1) / page_size + 2, Vmm::VirtualMemoryFlags::VMM_PRESENT);

            if (calculate_checksum(h, h->length) == 0) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with signature %s", h->signature);
                Terminal::write_line(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    }
}