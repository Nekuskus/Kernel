#include "acpi.hpp"

#include <stddef.h>

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/panic.hpp>
#include <system/terminal.hpp>

static Acpi::RsdpInfo rsdp_info{};
static auto acpi_tables = LinkedList<Acpi::SdtHeader *>();

inline uint8_t calculate_checksum(void *ptr, size_t size) {
    uint8_t sum = 0;

    for (size_t i = 0; i < size; i++)
        sum += ((uint8_t *)ptr)[i];

    return sum;
}

Acpi::SdtHeader *Acpi::get_table(const char *signature) {
    for (auto table : acpi_tables)
        if (!strncmp(table->signature, signature, 4))
            return table;

    return nullptr;
}

void Acpi::init(uint64_t rsdp) {
    rsdp_info.rsdp_address = rsdp + virtual_kernel_base;

    RsdpDescriptor *rsdp_ptr = (RsdpDescriptor *)rsdp_info.rsdp_address;
    rsdp_info.version = rsdp_ptr->revision + 1;
    rsdp_info.address = (uint64_t)rsdp_ptr->rsdt_address + virtual_physical_base;

    memcpy(rsdp_info.oem_id, rsdp_ptr->oem_id, 6);
    rsdp_info.oem_id[6] = '\0';

    char text[255] = "";

    sprintf(text, "[ACPI] Given ACPI with OEM ID '%s' and version %d.\n\r", rsdp_info.oem_id, rsdp_info.version);
    Debug::print(text);

    if (rsdp_info.version >= 2) {
        auto xsdt = (XsdtHeader *)rsdp_info.address;
        size_t entries = (xsdt->header.length - sizeof(SdtHeader)) / 8;

        for (size_t i = 0; i < entries; i++) {
            auto h = (SdtHeader *)(xsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_ctx_kernel(), h, (void *)xsdt->tables[i], 1, (int)Vmm::VirtualMemoryFlags::PRESENT);
            Vmm::map_pages(Vmm::get_ctx_kernel(), h, (void *)xsdt->tables[i], (h->length + 0x1000 - 1) / 0x1000, (int)Vmm::VirtualMemoryFlags::PRESENT);

            if (!calculate_checksum(h, h->length)) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with address %x and signature %c%c%c%c\n\r", xsdt->tables[i], h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Debug::print(text);
                Terminal::write(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    } else {
        auto rsdt = (RsdtHeader *)rsdp_info.address;
        size_t entries = (rsdt->header.length - sizeof(SdtHeader)) / 4;

        for (size_t i = 0; i < entries; i++) {
            auto h = (SdtHeader *)((uint64_t)rsdt->tables[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_ctx_kernel(), h, (void *)(uint64_t)rsdt->tables[i], 1, (int)Vmm::VirtualMemoryFlags::PRESENT);
            Vmm::map_pages(Vmm::get_ctx_kernel(), h, (void *)(uint64_t)rsdt->tables[i], (h->length + 0x1000 - 1) / 0x1000, (int)Vmm::VirtualMemoryFlags::PRESENT);

            if (!calculate_checksum(h, h->length)) {
                char text[255] = "";

                sprintf(text, "[ACPI] Found table with address %x and signature %c%c%c%c\n\r", (uint64_t)h - virtual_physical_base, h->signature[0], h->signature[1], h->signature[2], h->signature[3]);
                Debug::print(text);
                Terminal::write(text, 0xFFFFFF);
                acpi_tables.push_back(h);
            }
        }
    }
}