#include "idt.hpp"

#include <lib/lib.hpp>

#include "cpu/apic.hpp"
#include "debugging.hpp"
#include "drivers/port.hpp"
#include "terminal.hpp"

static Idt::Entry idt_entries[256] = {};
static Idt::InterruptHandler interrupt_handlers[256] = {};
static Idt::Pointer idt_pointer{};

struct Exception {
    const char *mnemonic;
    const char *message;
};

static constexpr inline Exception exceptions[] = {
    { .mnemonic = "DE", .message = "Division By Zero" },
    { .mnemonic = "DB", .message = "Debug" },
    { .mnemonic = "NMI", .message = "Non Maskable Interrupt" },
    { .mnemonic = "BP", .message = "Breakpoint" },
    { .mnemonic = "OF", .message = "Overflow" },
    { .mnemonic = "BR", .message = "Out of Bounds" },
    { .mnemonic = "UD", .message = "Invalid Opcode" },
    { .mnemonic = "NM", .message = "No Coprocessor" },
    { .mnemonic = "DF", .message = "Double Fault" },
    { .mnemonic = "-", .message = "Coprocessor Segment Overrun" },
    { .mnemonic = "TS", .message = "Invalid TSS" },
    { .mnemonic = "NP", .message = "Segment Not Present" },
    { .mnemonic = "SS", .message = "Stack Fault" },
    { .mnemonic = "GP", .message = "General Protection Fault" },
    { .mnemonic = "PF", .message = "Page Fault" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "MF", .message = "x87 Floating-Point Exception" },
    { .mnemonic = "AC", .message = "Alignment Check" },
    { .mnemonic = "MC", .message = "Machine Check" },
    { .mnemonic = "XM/XF", .message = "SIMD Floating-Point Exception" },
    { .mnemonic = "VE", .message = "Virtualization Exception" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "-", .message = "Reserved" },
    { .mnemonic = "SX", .message = "Security Exception" },
    { .mnemonic = "-", .message = "Reserved" },
};

extern "C" void *isrs[256];

extern "C" void isr_handler(Cpu::Registers *registers) {
    uint8_t n = registers->int_num & 0xFF;
    Idt::InterruptHandler *handler = &interrupt_handlers[n];

    if (n < 32) {
        char text[2048] = "";

        sprintf(text, "Received Exception #%s (%s) on CPU #%d:\n\rCPU registers: RIP: %x, RSP: %x\n\r    RAX: %x, RBX: %x, RCX: %x, RDX : %x\n\r    RSI: %x, RDI: %x, RSP: %x, RBP: %x\n\r    R8: %x, R9: %x, R10: %x, R11: %x\n\r    R12: %x, R12: %x, R13: %x, R14: %x\n\r    R15: %x\n\r", exceptions[n].mnemonic, exceptions[n].message, Cpu::get_current_cpu()->id, registers->rip, registers->rsp, registers->rax, registers->rbx, registers->rcx, registers->rbx, registers->rsi, registers->rdi, registers->rsp, registers->rbp, registers->r8, registers->r9, registers->r10, registers->r11, registers->r12, registers->r13, registers->r13, registers->r14, registers->r15);
        Terminal::write(text, 0xFFFFFF, 0xe50000);
        Debug::print(text);
    }

    if (handler->function)
        interrupt_handlers[n].function(registers);

    if (handler->is_irq)
        Cpu::Apic::LocalApic::send_eoi();

    if (!handler->should_iret && !handler->is_irq)
        while (1)
            ;
}

void Idt::register_interrupt_handler(uint16_t n, void (*function)(Cpu::Registers *), bool is_irq, bool should_iret) {
    interrupt_handlers[n] = {
        .function = function,
        .is_irq = is_irq,
        .should_iret = should_iret,
    };
}

void Idt::set_irq(uint16_t n, bool is_irq) {
    interrupt_handlers[n].is_irq = is_irq;
}

inline void set_entry(uint8_t vec, uintptr_t function, uint16_t selector, uint8_t ist) {
    idt_entries[vec] = {
        .offset_low = (uint16_t)(function & 0xFFFF),
        .selector = selector,
        .flags = (uint16_t)((ist & 0x7) | (1ULL << 9) | (1ULL << 10) | (1ULL << 11) | (1ULL << 15)),
        .offset_mid = (uint16_t)((function >> 16) & 0xFFFF),
        .offset_high = (uint32_t)((function >> 32) & 0xFFFFFFFF),
        .reserved = 0,
    };
}

inline void set_entry(uint8_t vec, uintptr_t function, uint16_t selector, uint8_t ist, uint8_t dpl) {
    idt_entries[vec] = {
        .offset_low = (uint16_t)(function & 0xFFFF),
        .selector = selector,
        .flags = (uint16_t)((ist & 0x7) | (1ULL << 9) | (1ULL << 10) | (1ULL << 11) | ((dpl & 0x3) << 13) | (1ULL << 15)),
        .offset_mid = (uint16_t)((function >> 16) & 0xFFFF),
        .offset_high = (uint32_t)((function >> 32) & 0xFFFFFFFF),
        .reserved = 0,
    };
}

void Idt::init() {
    Port::outb(0x20, 17);
    Port::outb(0xA0, 17);
    Port::outb(0x21, 32);
    Port::outb(0xA1, 40);
    Port::outb(0x21, 1);
    Port::outb(0xA1, 1);
    Port::outb(0x21, 4);
    Port::outb(0xA1, 2);
    Port::outb(0x21, 255);
    Port::outb(0xA1, 255);

    for (size_t i = 0; i < 249; i++)
        set_entry(i, (uintptr_t)isrs[i], 0x08, 0);

    set_entry(249, (uintptr_t)isrs[249], 0x08, 0, 3);

    for (size_t i = 250; i < 256; i++)
        set_entry(i, (uintptr_t)isrs[i], 0x08, 0);

    idt_pointer = { .limit = 256 * sizeof(Entry) - 1, .base = (uint64_t)&idt_entries };

    asm volatile("lidt %0" ::"m"(idt_pointer)
                 : "memory");

    char debug[256] = "";

    sprintf(debug, "[IDT] Initialized IDT on CPU #%d.\n\r", Cpu::get_current_cpu()->id);
    Debug::print(debug);
}