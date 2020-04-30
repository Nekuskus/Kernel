#include "time.hpp"

#include <lib/lib.hpp>
#include <system/cpu/apic.hpp>
#include <system/cpu/cpu.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/panic.hpp>

static Hpet::Hpet* hpet = nullptr;
static uint64_t clk = 0;

void Time::ksleep(uint64_t time) {
    uint64_t final_time = hpet->main_counter_value + (time * 1000000000000) / clk;

    while (hpet->main_counter_value < final_time)
        ;
}

void Hpet::init() {
    HpetTable* hpet_table = (HpetTable*)Acpi::get_table("HPET");
    
    if (!hpet_table)
        panic("UNSUPPORTED_HARDWARE_MISSING_HPET");

    hpet = (Hpet*)(hpet_table->address + virtual_physical_base);

    clk = (hpet->general_capabilities >> 32) & 0xFFFFFFFF;

    hpet->general_configuration = 0;
    hpet->main_counter_value = 0;
    hpet->general_configuration = 1;

    Debug::print("[HPET] Successfully initialized.\n\r");
}