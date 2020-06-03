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
    if (!rsdp)
        panic("Your hardware is unsupported; ACPI is missing on your computer, that's little weird, usually computers have that.");

    rsdp_info.rsdp_address = rsdp + virtual_kernel_base;

    RsdpDescriptor *rsdp_ptr = (RsdpDescriptor *)rsdp_info.rsdp_address;
    rsdp_info.version = rsdp_ptr->revision + 1;
    rsdp_info.address = (uint64_t)rsdp_ptr->rsdt_address + virtual_physical_base;

    memcpy(rsdp_info.oem_id, rsdp_ptr->oem_id, 6);
    rsdp_info.oem_id[6] = '\0';

    char text[255] = "";

    sprintf(text, "[ACPI] Given ACPI v%d by OEM '%s':\n\r", rsdp_info.version, rsdp_info.oem_id);
    Debug::print(text);
    Terminal::write(text, 0xFFFFFF);

    auto rsdt = (RsdtHeader *)rsdp_info.address;
    size_t entries = (rsdt->header.length - sizeof(SdtHeader)) / (rsdp_info.version < 2 ? 4 : 8);
    auto ctx = Vmm::get_ctx_kernel();
    int flags = (int)Vmm::VirtualMemoryFlags::PRESENT;

    for (size_t i = 0; i < entries; i++) {
        auto h = (SdtHeader *)((uint64_t)rsdt->tables[i] + virtual_physical_base);
        auto h_phys = (void *)(uint64_t)rsdt->tables[i];

        Vmm::map_pages(ctx, h, h_phys, 1, flags);
        Vmm::map_pages(ctx, h, h_phys, (h->length + 0x1000 - 1) / 0x1000, flags);

        if (!calculate_checksum(h, h->length)) {
            sprintf(text, "[ACPI] Found table '%c%c%c%c' at %x.\n\r", h->signature[0], h->signature[1], h->signature[2], h->signature[3], h_phys);
            Debug::print(text);
            Terminal::write(text, 0xFFFFFF);

            acpi_tables.push_back(h);
        }
    }

    Debug::print("[ACPI] Finished setting up.");
    Terminal::write("[ACPI] Finished setting up.", 0xFFFFFF);
}