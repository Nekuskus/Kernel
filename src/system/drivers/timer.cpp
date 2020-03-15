#include "timer.hpp"
#include <panic.hpp>
#include <system/mm/mm.hpp>
#include <system/cpu/apic.hpp>
#include <system/idt.hpp>

Hpet::Hpet* hpet;

uint64_t uptime_raw = 0, uptime_sec = 0, epoch = 0;

void tick_handler([[maybe_unused]] const Idt::InterruptRegisters* registers) {
    if (!(++uptime_raw % 1000)) {
        uptime_sec++;
        epoch++;
    }
}

void Timer::ksleep(uint64_t time) {
    uint64_t final_time = uptime_raw + time;

    final_time++;

    while (uptime_raw < final_time);
}

void Hpet::init() {
    HpetTable* hpet_table = (HpetTable*)Acpi::get_table("HPET");
    hpet = (Hpet*)(hpet_table->address + virtual_physical_base);

    if (!hpet_table)
        panic("Unsupported hardware configuration; HPET is missing from your system.");

    if (!(hpet->general_capabilities & (1 << 15)))
        panic("HPET is not capable of legacy replacement");

    hpet->general_configuration |= 0b10;
    hpet->main_counter_value = 0;
    
    if (!(hpet->timers[0].config_and_capabilities & (1 << 4)))
        panic("HPET timer #0 does not support periodic mode");

    hpet->timers[0].config_and_capabilities |= (1 << 2) | (1 << 3) | (1 << 6);
    hpet->timers[0].comparator_value = 1000000000000000 / (hpet->general_capabilities >> 32);
    hpet->general_configuration |= 0b01;

    Idt::register_interrupt_handler(0, tick_handler, true, true);
    Apic::IoApic::unmask_irq(0);
}