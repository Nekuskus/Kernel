#include "exceptions.hpp"

#include <stdint.h>

#include <lib/lib.hpp>

#include "cpu/cpu.hpp"
#include "debugging.hpp"
#include "idt.hpp"
#include "terminal.hpp"

void page_fault_handler(const Cpu::Registers* registers) {
    uint64_t cr2;

    asm volatile("mov %%cr2, %0"
                 : "=r"(cr2));

    char text[255] = "";

    sprintf(text, "    Linear address: %x, Condition:\n\r    %s%s%s%s%s%s%s\n\r", cr2, registers->error_code & (1ULL << 0) ? "Page Level protection violation, " : "Non-present page, ", registers->error_code & (1ULL << 1) ? "Write, " : "Read, ", registers->error_code & (1ULL << 2) ? "User access, " : "Supervisor access, ", registers->error_code & (1ULL << 3) ? "Reserved page, " : "", registers->error_code & (1ULL << 4) ? "Instruction fetch, " : "", registers->error_code & (1ULL << 5) ? "Protection key violation, " : "", registers->error_code & (1ULL << 15) ? "SGX violation" : "");
    Terminal::write(text, 0xFFFFFF, 0xe50000);
    Debug::print(text);
}

void general_protection_fault_handler(const Cpu::Registers* registers) {
    char text[255] = "";

    sprintf(text, "    Segment selector index: %i.\n\r", registers->error_code);
    Terminal::write(text, 0xFFFFFF, 0xe50000);
    Debug::print(text);
}

void Exceptions::init() {
    Idt::register_interrupt_handler(13, general_protection_fault_handler, false, false);
    Idt::register_interrupt_handler(14, page_fault_handler, false, false);

    char debug[256] = "";
    sprintf(debug, "[Exceptions] Initialized exception handlers on CPU #%d.\n\r", Cpu::get_current_cpu()->id);
    Debug::print(debug);
}