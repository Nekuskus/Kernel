#include "panic.hpp"

#include <lib/lib.hpp>

#include "cpu/cpu.hpp"
#include "terminal.hpp"

void panic(const char* message) {
    char text[8192] = "";

    sprintf(text, ":( Kernel Panic Occurred,\n\rError message: %s", message);
    Terminal::write_line(text, 0xFFFFFF, 0xe50000);
    Cpu::halt_forever();
}

void panic(const char* message, Idt::InterruptRegisters* registers) {
    char text[8192] = "";

    sprintf(text, ":( Kernel Panic Occurred,\n\rError message: %s\n\rCPU registers: RIP: %x, RSP: %x\n\r    RAX: %x, RBX: %x, RCX: %x, RDX : %x\n\r    RSI: %x, RDI: %x, RSP: %x, RBP: %x\n\r    R8: %x, R9: %x, R10: %x, R11: %x\n\r    R12: %x, R12: %x, R13: %x, R14: %x\n\r    R15: %x", message, registers->rip, registers->rsp, registers->rax, registers->rbx, registers->rcx, registers->rbx, registers->rsi, registers->rdi, registers->rsp, registers->rbp, registers->r8, registers->r9, registers->r10, registers->r11, registers->r12, registers->r13, registers->r13, registers->r14, registers->r15);
    Terminal::write_line(text, 0xFFFFFF, 0xe50000);
    Cpu::halt_forever();
}