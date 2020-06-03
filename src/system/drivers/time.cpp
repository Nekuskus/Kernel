#include "time.hpp"

#include <lib/lib.hpp>
#include <system/cpu/apic.hpp>
#include <system/cpu/cpu.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/panic.hpp>

static Hpet::Hpet *hpet = nullptr;
static size_t clk = 0;

void Time::ksleep(size_t ms) {
    size_t target = hpet->main_counter_value + (ms * 1000000000000) / clk;

    while (hpet->main_counter_value < target)
        asm volatile("pause");
}

void Hpet::init() {
    auto hpet_table = (HpetTable *)Acpi::get_table("HPET");

    if (!hpet_table)
        panic("This is akward; This computer is missing HPET from the ACPI tables, usually computers have this, make sure you didn't disable it in your BIOS settings!");

    hpet = (Hpet *)(hpet_table->address + virtual_physical_base);
    clk = (hpet->general_capabilities >> 32) & 0xFFFFFFFF;

    hpet->general_configuration = 0;
    hpet->main_counter_value = 0;
    hpet->general_configuration = 1;

    Debug::print("[HPET] Successfully initialized.\n\r");
}