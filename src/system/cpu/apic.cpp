#include "apic.hpp"

#include <cpuid.h>

#include <lib/lib.hpp>
#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/mm/mm.hpp>

#include "cpu.hpp"

static size_t lapic_base = 0;

uint32_t Cpu::Apic::LocalApic::read(uint32_t reg) {
    return *((volatile uint32_t *)(lapic_base + reg));
}

void Cpu::Apic::LocalApic::write(uint32_t reg, uint32_t data) {
    *((volatile uint32_t *)(lapic_base + reg)) = data;
}

void Cpu::Apic::LocalApic::send_eoi() {
    write(0xB0, 0);
}

void Cpu::Apic::LocalApic::send_ipi(uint32_t lapic, uint32_t vector) {
    write(lapic_icr1, lapic << 24);
    write(lapic_icr0, vector);
}

void Cpu::Apic::LocalApic::set_timer_mask(bool mask) {
    auto entry = read((uint32_t)TimerRegisters::LVT_TIMER);

    if (mask)
        entry |= 1UL << 16;
    else
        entry &= ~(1UL << 16);

    write((uint32_t)TimerRegisters::LVT_TIMER, entry);
}

void Cpu::Apic::LocalApic::enable_timer() {
    write((uint32_t)TimerRegisters::DIVIDE_CONFIG, 0x3);
    write((uint32_t)TimerRegisters::INITIAL_COUNT, 0xFFFFFFFF);

    set_timer_mask(false);
    Time::ksleep(10);
    set_timer_mask(true);

    size_t ticks_per_ms = (0xFFFFFFFF - (size_t)read((uint32_t)TimerRegisters::CURRENT_COUNT)) / 10;
    auto entry = read((uint32_t)TimerRegisters::LVT_TIMER);
    entry &= ~(0b11 << 17);
    entry |= 1 << 17;
    entry = (entry & 0xFFFFFF00) | 128;

    write((uint32_t)TimerRegisters::LVT_TIMER, entry);
    write((uint32_t)TimerRegisters::DIVIDE_CONFIG, 0x3);
    write((uint32_t)TimerRegisters::INITIAL_COUNT, (uint32_t)ticks_per_ms);
    set_timer_mask(false);
}

void Cpu::Apic::LocalApic::init() {
    write(0xF0, read(0xF0) | 0x1FF);

    char text[255] = "";
    sprintf(text, "[Local APIC] Initialized for CPU #%d.\n\r", Cpu::get_current_cpu()->id);
    Debug::print(text);
}

uint32_t Cpu::Apic::IoApic::read(uint64_t ioapic_base, uint32_t reg) {
    volatile uint32_t *base = (volatile uint32_t *)(ioapic_base + virtual_physical_base);
    *base = reg;
    return *(base + 4);
}

void Cpu::Apic::IoApic::write(uint64_t ioapic_base, uint32_t reg, uint32_t data) {
    volatile uint32_t *base = (volatile uint32_t *)(ioapic_base + virtual_physical_base);
    *base = reg;
    *(base + 4) = data;
}

uint32_t Cpu::Apic::IoApic::get_max_redirect(uint64_t base) {
    return (read(base, 1) & 0xFF0000) >> 16;
}

Madt::IoApic *Cpu::Apic::IoApic::from_redirect(uint32_t gsi) {
    for (auto ioapic : Madt::ioapics)
        if (ioapic->gsi_base <= gsi && ioapic->gsi_base + get_max_redirect(ioapic->ioapic_base) > gsi)
            return ioapic;

    return nullptr;
}

void Cpu::Apic::init() {
    lapic_base = Madt::madt->apic_address + virtual_physical_base;
    LocalApic::init();
    Debug::print("[APIC] Initialized.\n\r");
}