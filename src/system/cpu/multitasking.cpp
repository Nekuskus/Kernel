#include "multitasking.hpp"

#include <lib/linked_list.hpp>
#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/idt.hpp>
#include <system/terminal.hpp>

#include "apic.hpp"

auto processes = LinkedList<Cpu::Multitasking::Process*>();

Cpu::Multitasking::Thread* Cpu::Multitasking::find_next_thread() {
    return nullptr;
}

void Cpu::Multitasking::switch_task() {
    
}

void schedule([[maybe_unused]] const Cpu::Registers* registers) {
    Cpu::Multitasking::switch_task();
}

void Cpu::Multitasking::init() {
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_LVT, 232 | (1 << 17));
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_DIV, 0x3);
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, 0xFFFFFFFF);

    Time::ksleep(20);

    uint32_t ticks = 0xFFFFFFFF - Apic::LocalApic::read(Apic::LocalApic::TimerRegisters::TIMER_CURRCNT);

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, ticks / 10);

    Idt::register_interrupt_handler(232, schedule, true, true);
    Debug::print("[MULTITASKING] Initialized scheduler.\n");
}