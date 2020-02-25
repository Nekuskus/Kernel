#include "exceptions.hpp"

#include <stdint.h>

#include <lib/lib.hpp>

#include "debugging.hpp"
#include "idt.hpp"
#include "terminal.hpp"

void page_fault_handler(const Idt::InterruptRegisters* registers) {
    uint64_t cr2;

    asm volatile("mov %%cr2, %0"
                 : "=r"(cr2));

    char text[255] = "";

    sprintf(text, "    Linear address: %x, Condition:\n\r    %s%s%s%s%s%s%s", cr2, registers->error_code & (1ULL << 0) ? "Page Level protection violation, " : "Non-present page, ", registers->error_code & (1ULL << 1) ? "Write, " : "Read, ", registers->error_code & (1ULL << 2) ? "User access, " : "Supervisor access, ", registers->error_code & (1ULL << 3) ? "Reserved page, " : "", registers->error_code & (1ULL << 4) ? "Instruction fetch, " : "", registers->error_code & (1ULL << 5) ? "Protection key violation, " : "", registers->error_code & (1ULL << 15) ? "SGX violation" : "");
    Terminal::write_line(text, 0xFFFFFF, 0xe50000);
}

void general_protection_fault_handler(const Idt::InterruptRegisters* registers) {
    char text[255] = "";

    sprintf(text, "    Segment selector index: %i.", registers->error_code);
    Terminal::write_line(text, 0xFFFFFF, 0xe50000);
}

void Exceptions::init() {
    Idt::register_interrupt_handler(13, general_protection_fault_handler, false, false);
    Idt::register_interrupt_handler(14, page_fault_handler, false, false);

    Debug::print("[CPUEXCP] Initialized exception handlers.\n");
}