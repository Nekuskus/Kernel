#include "multitasking.hpp"

#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/idt.hpp>
#include <system/terminal.hpp>

#include "apic.hpp"

void schedule([[maybe_unused]] const Idt::InterruptRegisters* registers) {
    Terminal::write('.', 0xFFFFFF);
}

void Cpu::Multitasking::init() {
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_LVT, 232 | (1 << 17));
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_DIV, 0x3);
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, 0xFFFFFFFF);

    Time::ksleep(10);

    uint32_t ticks = 0xFFFFFFFF - Apic::LocalApic::read(Apic::LocalApic::TimerRegisters::TIMER_CURRCNT);

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, ticks / 10);

    Idt::register_interrupt_handler(232, schedule, true, true);
    Debug::print("[MULTITASKING] Initialized scheduler.\n");
}