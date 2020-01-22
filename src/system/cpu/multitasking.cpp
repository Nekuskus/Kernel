#include "multitasking.hpp"
#include <system/acpi/apic.hpp>
#include <system/idt.hpp>
#include <system/port.hpp>
#include <system/terminal.hpp>

void schedule([[maybe_unused]] const Idt::InterruptRegisters* registers) {
    Terminal::write('.', 0xFFFFFF);
}

void Cpu::Multitasking::init() {
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_LVT, 32 | 1);
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_DIV, 0x3);

    uint32_t divider = 1193182 / 100;

    Port::outb(0x43, 54);
    Port::outb(0x40, (uint8_t)(divider & 0xFF));
    Port::outb(0x40, (uint8_t)(divider >> 8));

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, 0xFFFFFFFF);

    Port::outb(0x21, Port::inb(0x21) & ~1);

    //while (!pit_ticked)
        //;

    Idt::register_interrupt_handler(32, schedule, true, true);

    Port::outb(0x21, Port::inb(0x21) & 1);

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_LVT, 16);

    uint32_t ticks = read(Apic::LocalApic::TimerRegisters::TIMER_CURRCNT) - 0xFFFFFFFF;

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, ticks);
    Apic::IoApic::unmask_irq(0);
}