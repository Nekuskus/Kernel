#include <cpuid.h>
#include <hardware/acpi/acpi.hpp>
#include <hardware/acpi/apic.hpp>
#include <hardware/cpu/smp/smp.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/vmm.hpp>
#include <hardware/panic.hpp>
#include <hardware/port.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>

Spark::Acpi::RsdpInfo rsdp_info;
Spark::Acpi::MadtHeader* madt;

inline uint8_t bios_calculate_checksum(void* ptr, size_t size) {
    uint8_t sum = 0;

    for (size_t i = 0; i < size; i++)
        sum += ((uint8_t*)ptr)[i];

    return sum;
}

inline Spark::Acpi::RsdpInfo bios_detect_rsdp(uint64_t base, size_t length) {
    uint64_t address = base + virtual_physical_base;
    Spark::Acpi::RsdpInfo info{};

    for (size_t off = 0; off < length; off += 16) {
        Spark::Acpi::RsdpDescriptor* rsdp = (Spark::Acpi::RsdpDescriptor*)(address + off);

        Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), rsdp, (void*)((uint64_t)rsdp - virtual_physical_base), 1, 0);

        if (strncmp(rsdp->signature, "RSD PTR ", 8) || bios_calculate_checksum(rsdp, sizeof(Spark::Acpi::RsdpDescriptor)))
            continue;

        info.rsdp_address = address + off;

        for (size_t i = 0; i < 6; i++)
            info.oem_id[i] = rsdp->oem_id[i];

        info.oem_id[6] = '\0';

        if (!rsdp->revision) {
            info.version = 1;
            info.address = (uint64_t)rsdp->rsdt_address + virtual_physical_base;

            Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), rsdp, (void*)((uint64_t)rsdp - virtual_physical_base), (((Spark::Acpi::RsdtHeader*)info.address)->header.length + 0x1000 - 1) / 0x1000, 0);
            break;
        } else {
            Spark::Acpi::RsdpDescriptor2* rsdp2 = (Spark::Acpi::RsdpDescriptor2*)rsdp;

            Spark::Vmm::map_pages(Spark::Vmm::get_current_context(), rsdp, (void*)((uint64_t)rsdp2 - virtual_physical_base), (((Spark::Acpi::XsdtHeader*)(rsdp2->xsdt_address + virtual_physical_base))->header.length + 0x1000 - 1) / 0x1000, 0);

            if (bios_calculate_checksum(rsdp2, sizeof(Spark::Acpi::RsdpDescriptor)))
                continue;

            info.version = 2;
            info.address = (uint64_t)rsdp2->xsdt_address + virtual_physical_base;
            break;
        }
    }

    return info;
}

inline Spark::Acpi::RsdpInfo bios_detect_rsdp() {
    uint16_t* ebda_seg_ptr = (uint16_t*)(0x40E + virtual_physical_base);
    Spark::Acpi::RsdpInfo info = bios_detect_rsdp(*ebda_seg_ptr << 4, 0x400);

    if (!info.version)
        info = bios_detect_rsdp(0xE0000, 0x20000);
    if (!info.version)
        Spark::panic("ACPI not supported");

    return info;
}

Spark::Acpi::SdtHeader* Spark::Acpi::get_table(const char* signature) {
    if (!rsdp_info.version)
        return nullptr;

    if (rsdp_info.version >= 2) {
        Spark::Acpi::XsdtHeader* xsdt = (Spark::Acpi::XsdtHeader*)rsdp_info.address;
        uint32_t entries = (xsdt->header.length - sizeof(xsdt->header)) / 4;

        for (uint32_t i = 0; i < entries; i++) {
            Spark::Acpi::SdtHeader* h = (Spark::Acpi::SdtHeader*)((uint64_t)xsdt->other_std[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), h, (void*)(h - virtual_physical_base), 1, 0);
            Vmm::map_pages(Vmm::get_current_context(), h, (void*)(h - virtual_physical_base), (h->length + 0x1000 - 1) / 0x1000, 0);

            if (!strncmp(h->signature, signature, 4))
                return h;
        }
    } else {
        Spark::Acpi::RsdtHeader* rsdt = (Spark::Acpi::RsdtHeader*)rsdp_info.address;
        uint32_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 8;

        for (uint32_t i = 0; i < entries; i++) {
            Spark::Acpi::SdtHeader* h = (Spark::Acpi::SdtHeader*)((uint64_t)rsdt->other_std[i] + virtual_physical_base);

            Vmm::map_pages(Vmm::get_current_context(), h, (void*)(h - virtual_physical_base), 1, 0);
            Vmm::map_pages(Vmm::get_current_context(), h, (void*)(h - virtual_physical_base), (h->length + 0x1000 - 1) / 0x1000, 0);

            if (!strncmp(h->signature, signature, 4))
                return h;
        }
    }

    return nullptr;
}

void Spark::Acpi::init() {
    rsdp_info = bios_detect_rsdp();
    char text[255] = "";

    sprintf(text, "[ACPI] Detected ACPI with OEM ID %s and version %d", rsdp_info.oem_id, rsdp_info.version);
    Terminal::write_line(text, 0xFFFFFF);

    madt = (MadtHeader*)get_table("APIC");
    size_t table_size = madt->header.length - sizeof(MadtHeader);
    uint64_t list = (uint64_t)madt + sizeof(MadtHeader), offset = 0;

    Apic::LocalApic::init();
    Cpu::Smp::init();

    while (offset < table_size) {
        InterruptController* interrupt_controller = (InterruptController*)(list + offset);

        if (interrupt_controller->type == InterruptControllerType::LAPIC) {
            LocalApic* cpu = (LocalApic*)interrupt_controller;

            if (!(cpu->flags & 1))
                continue;

            uint32_t a, b, c, d;
            __cpuid(1, a, b, c, d);

            Cpu::Smp::CpuEntry cpu_entry{
                .lapic_id = cpu->id,
                .bsp = ((b >> 24) & 0xFF) == cpu->id
            };

            Cpu::Smp::boot_cpu(cpu_entry);
        }

        offset += interrupt_controller->length;
    }
}