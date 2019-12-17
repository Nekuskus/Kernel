#pragma once
#include <stdint.h>

namespace Spark::Idt {
    struct [[gnu::packed]] InterruptRegisters {
        uint64_t ds;
        uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, useless, rbx, rdx, rcx, rax;
        uint64_t int_num, error_code;
        uint64_t rip, cs, rflags, rsp, ss;
    };

    struct [[gnu::packed]] Entry {
        uint16_t offset_low;
        uint16_t selector;
        uint16_t flags;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t reserved;
    };

    struct [[gnu::packed]] Pointer {
        uint16_t limit;
        uint64_t base;
    };

    using idt_function = void (*)(InterruptRegisters*);

    struct InterruptHandler {
        idt_function handler;
        bool is_irq;
        bool should_iret;
    };

    void init();
    void register_interrupt_handler(uint16_t n, idt_function function, bool is_irq, bool should_iret);
};  // namespace Spark::Idt