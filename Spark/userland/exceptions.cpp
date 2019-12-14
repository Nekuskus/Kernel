#include <hardware/idt.hpp>
#include <hardware/panic.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>
#include <userland/exceptions.hpp>

extern "C" void page_fault_handler();

extern "C" void idt_page_fault(Spark::Exceptions::CpuFaultState* state) {
    uint64_t cr2;
    asm("mov %%cr2, %0"
        : "=r"(cr2));
    char text[255] = "";

    sprintf(text, "Page Fault occurred at %x, Conditions:\n\r    %s%s%s%s%s%s%s", cr2, state->error_code << 0 ? "Page Level protection violation, " : "Non-present page, ", state->error_code << 1 ? "Write, " : "Read, ", state->error_code << 2 ? "User access, " : "Supervisor access, ", state->error_code << 3 ? "Reserved page, " : "", state->error_code << 4 ? "Instruction fetch, " : "", state->error_code << 5 ? "Protection key violation, " : "", state->error_code << 0 ? "SGX violation" : "");

    if (!(state->error_code << 2))
        Spark::panic(text);

    Spark::Terminal::write_line(text, 0xFFFFFF, 0xe50000);
    asm("cli; hlt");
}

void Spark::Exceptions::init() {
    Idt::set_gate(14, (uintptr_t)page_fault_handler, 0x08, 0x8E);
}