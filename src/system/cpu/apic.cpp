#include "apic.hpp"

#include <cpuid.h>

#include <system/acpi/acpi.hpp>
#include <system/cpu/cpu.hpp>
#include <system/cpu/smp/smp.hpp>
#include <system/debugging.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/msr.hpp>

uint64_t lapic_base = 0;

uint32_t Cpu::Apic::LocalApic::read(uint32_t reg) {
    uint32_t* value = (uint32_t*)(lapic_base + reg);

    return *value;
}

void Cpu::Apic::LocalApic::write(uint32_t reg, uint32_t data) {
    uint32_t* value = (uint32_t*)(lapic_base + reg);
    *value = data;
}

void Cpu::Apic::LocalApic::init() {
    uint64_t apic_msr_base = Msr::read(apic_base);
    lapic_base = (apic_msr_base & 0xFFFFFFFFFFFFF000) + virtual_kernel_base;
    apic_msr_base |= 1 << 11;

    Vmm::map_pages(Vmm::get_ctx_kernel(), (void*)lapic_base, (void*)(apic_msr_base & 0xFFFFFFFFFFFFF000), 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);
    Msr::write(apic_base, apic_msr_base);
    write(lapic_tpr, 0);
    write(0xF0, 0xFF | 0x100);

    Debug::print("[APIC] Initialized LAPIC.\n");
}

void Cpu::Apic::LocalApic::send_ipi(uint32_t target, uint32_t flags) {
    write(icr_high, target << 24);
    write(icr_low, flags);

    while (read(icr_low) & IcrFlags::PENDING)
        ;
}

void Cpu::Apic::LocalApic::send_eoi() {
    write(0xB0, 0);
}

uint32_t Cpu::Apic::IoApic::read(uint32_t ioapic_base, uint32_t reg) {
    uint32_t* ioapic = (uint32_t*)((uint64_t)ioapic_base + virtual_kernel_base);
    ioapic[0] = reg & 0xFF;

    return ioapic[4];
}

void Cpu::Apic::IoApic::write(uint32_t ioapic_base, uint32_t reg, uint32_t data) {
    uint32_t* ioapic = (uint32_t*)((uint64_t)ioapic_base + virtual_kernel_base);
    ioapic[0] = reg & 0xFF;
    ioapic[4] = data;
}

uint64_t Cpu::Apic::IoApic::read_entry(uint8_t gsi) {
    for (auto ioapic : Madt::get_ioapics()) {
        if (ioapic->gsi_base > gsi && ((read(ioapic->ioapic_base, ioapic_ver) >> 16) & 0xFF) + 1 + ioapic->gsi_base <= gsi)
            continue;

        uint8_t offset = 0x10 + gsi * 2;

        return read(ioapic->ioapic_base, offset) | ((uint64_t)read(ioapic->ioapic_base, offset + 1) << 32);
    }

    return 0;
}

uint64_t Cpu::Apic::IoApic::read_entry(Madt::IoApic* ioapic, uint8_t gsi) {
    if (ioapic->gsi_base > gsi && ((read(ioapic->ioapic_base, ioapic_ver) >> 16) & 0xFF) + 1 + ioapic->gsi_base <= gsi)
        return 0;

    uint8_t offset = 0x10 + gsi * 2;

    return read(ioapic->ioapic_base, offset) | ((uint64_t)read(ioapic->ioapic_base, offset + 1) << 32);
}

void Cpu::Apic::IoApic::set_entry(uint8_t gsi, uint64_t data) {
    for (auto ioapic : Madt::get_ioapics()) {
        if (ioapic->gsi_base > gsi && ((read(ioapic->ioapic_base, ioapic_ver) >> 16) & 0xFF) + 1 + ioapic->gsi_base <= gsi)
            continue;

        uint8_t offset = 0x10 + gsi * 2;

        write(ioapic->ioapic_base, offset, data & 0xFFFFFFFF);
        write(ioapic->ioapic_base, offset + 1, (data >> 32) & 0xFFFFFFFF);

        return;
    }
}

void Cpu::Apic::IoApic::set_entry(Madt::IoApic* ioapic, uint8_t gsi, uint64_t data) {
    if (ioapic->gsi_base > gsi && ((read(ioapic->ioapic_base, ioapic_ver) >> 16) & 0xFF) + 1 + ioapic->gsi_base <= gsi)
        return;

    uint8_t offset = 0x10 + gsi * 2;

    write(ioapic->ioapic_base, offset, data & 0xFFFFFFFF);
    write(ioapic->ioapic_base, offset + 1, (data >> 32) & 0xFFFFFFFF);
}

void Cpu::Apic::IoApic::set_entry(uint8_t gsi, uint8_t vector, DeliveryMode delivery_mode, DestinationMode destination_mode, uint16_t flags, uint32_t destination) {
    uint64_t data = vector | (delivery_mode << 8) | (destination_mode << 11);

    if (flags & 2)
        data |= 1 << 13;

    if (flags & 8)
        data |= 1 << 15;

    data |= (uint64_t)destination << 56;

    set_entry(gsi, data);
}

void Cpu::Apic::IoApic::set_entry(Madt::IoApic* ioapic, uint8_t gsi, uint8_t vector, DeliveryMode delivery_mode, DestinationMode destination_mode, uint16_t flags, uint32_t destination) {
    uint64_t data = vector | (delivery_mode << 8) | (destination_mode << 11);

    if (flags & 2)
        data |= 1 << 13;

    if (flags & 8)
        data |= 1 << 15;

    data |= (uint64_t)destination << 56;

    set_entry(ioapic, gsi, data);
}

void Cpu::Apic::IoApic::mask_gsi(uint32_t gsi) {
    for (auto ioapic : Madt::get_ioapics()) {
        if (ioapic->gsi_base > gsi && ((read(ioapic->ioapic_base, ioapic_ver) >> 16) & 0xFF) + 1 + ioapic->gsi_base <= gsi)
            continue;

        set_entry(ioapic, gsi - ioapic->gsi_base, read_entry(gsi - ioapic->gsi_base) | (1 << 16));

        return;
    }
}

void Cpu::Apic::IoApic::unmask_gsi(uint32_t gsi) {
    for (auto ioapic : Madt::get_ioapics()) {
        if (ioapic->gsi_base > gsi && ((read(ioapic->ioapic_base, ioapic_ver) >> 16) & 0xFF) + 1 + ioapic->gsi_base <= gsi)
            continue;

        set_entry(ioapic, gsi - ioapic->gsi_base, read_entry(gsi - ioapic->gsi_base) & ~(1 << 16));

        return;
    }
}

void Cpu::Apic::IoApic::mask_irq(uint32_t irq) {
    for (auto iso : Madt::get_isos()) {
        if (iso->source == irq) {
            mask_gsi(iso->gsi);

            return;
        }
    }

    mask_gsi(irq);
}

void Cpu::Apic::IoApic::unmask_irq(uint32_t irq) {
    for (auto iso : Madt::get_isos()) {
        if (iso->source == irq) {
            unmask_gsi(iso->gsi);

            return;
        }
    }

    unmask_gsi(irq);
}

void Cpu::Apic::IoApic::init() {
    if (!Madt::has_legacy_pic())
        return;

    for (uint8_t i = 0; i < 16; i++) {
        if (i == 2)
            continue;

        bool found = false;

        for (auto iso : Madt::get_isos()) {
            if (iso->source == i) {
                set_entry(iso->gsi, iso->source + 0x20, DeliveryMode::FIXED, DestinationMode::PHYSICAL, iso->flags, Cpu::get_current_cpu()->id);
                mask_gsi(iso->gsi);

                found = true;

                break;
            }
        }

        if (!found) {
            set_entry(i, i + 0x20, DeliveryMode::FIXED, DestinationMode::PHYSICAL, 0, Cpu::get_current_cpu()->id);
            mask_gsi(i);
        }

        Idt::set_irq(i + 0x20, true);
    }

    Debug::print("[APIC] Initialized IOAPIC.\n");
}