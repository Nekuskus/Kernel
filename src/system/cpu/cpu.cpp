#include "cpu.hpp"

#include <cpuid.h>

auto cpu_states = LinkedList<Cpu::CpuState*>();

bool Cpu::check_msr(uint32_t flag) {
    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);

    return d & flag;
}

void Cpu::halt_forever() {
    asm volatile(
        "cli\n"
        "1:\n"
        "hlt\n"
        "jmp 1b"
        :
        :
        : "memory");
}

LinkedList<Cpu::CpuState*>& Cpu::get_cpu_states() {
    return cpu_states;
}

void Cpu::push(Cpu::CpuState* cpu_state) {
    cpu_states.push_back(cpu_state);
}

Cpu::CpuState* Cpu::get_current_cpu() {
    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);

    uint32_t lapic_id = (b >> 24) & 0xFF;

    if (cpu_states.length() <= 0) {
        auto current_cpu = new CpuState;
        current_cpu->id = lapic_id;
        current_cpu->thread = -1;
        current_cpu->process = -1;
        current_cpu->booted = true;

        push(current_cpu);

        return current_cpu;
    }

    for (auto cpu_state : cpu_states)
        if (cpu_state->id == lapic_id)
            return cpu_state;

    return nullptr;
}