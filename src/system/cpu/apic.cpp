#include "apic.hpp"

#include <cpuid.h>

#include <system/acpi/madt.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>

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

void Cpu::Apic::init() {
    lapic_base = Madt::madt->apic_address + virtual_physical_base;
    LocalApic::write(0xF0, LocalApic::read(0xF0) | 0x1FF);
    Debug::print("[APIC] Initialized.\n\r");
}