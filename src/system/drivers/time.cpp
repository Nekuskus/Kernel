#include "time.hpp"

#include <system/cpu/apic.hpp>
#include <system/debugging.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/panic.hpp>

Hpet::Hpet* hpet;

volatile uint64_t uptime_raw = 0, uptime_sec = 0, epoch = 0;

void tick_handler([[maybe_unused]] const Idt::InterruptRegisters* registers) {
    if (!(++uptime_raw % 1000)) {
        uptime_sec++;
        epoch++;
    }
}

void Time::ksleep(uint64_t time) {
    uint64_t final_time = uptime_raw + time + 1;

    while (uptime_raw < final_time)
        ;
}

void Hpet::init() {
    HpetTable* hpet_table = (HpetTable*)Acpi::get_table("HPET");
    hpet = (Hpet*)(hpet_table->address + virtual_physical_base);

    if (!hpet_table)
        panic("UNSUPPORTED_HARDWARE_MISSING_HPET");

    if (!(hpet->general_capabilities & (1 << 15)))
        panic("HPET_LEGACY_REPLACEMENT_UNSUPPORTED");

    uint64_t counter_clk_period = hpet->general_capabilities >> 32, frequency = 1000000000000000 / counter_clk_period;

    hpet->general_configuration |= 0b10;
    hpet->main_counter_value = 0;

    if (!(hpet->timers[0].config_and_capabilities & (1 << 4)))
        panic("HPET_TIMER_0_PERIODIC_UNSUPPORTED");

    hpet->timers[0].config_and_capabilities |= (1 << 2) | (1 << 3) | (1 << 6);
    hpet->timers[0].comparator_value = frequency / 1000;
    hpet->general_configuration |= 0b01;

    Idt::register_interrupt_handler(32, tick_handler, true, true);
    Cpu::Apic::IoApic::unmask_irq(0);
    Debug::print("[HPET] Successfully initialized.\n");
}