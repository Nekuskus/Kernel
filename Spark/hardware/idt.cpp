#include <hardware/devices/vbe.hpp>
#include <hardware/idt.hpp>
#include <hardware/port.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>

Spark::Idt::Entry idt_entries[256]{};
Spark::Idt::Pointer idt_pointer{};

void Spark::Idt::init() {
    Port::outb(0x20, 0x11);
    Port::wait();
    Port::outb(0xA0, 0x11);
    Port::wait();
    Port::outb(0x21, 32);
    Port::wait();
    Port::outb(0xA1, 40);
    Port::wait();
    Port::outb(0x21, 4);
    Port::wait();
    Port::outb(0xA1, 2);
    Port::wait();
    Port::outb(0x21, 0xff);
    Port::wait();
    Port::outb(0xA1, 0xff);
    Port::wait();
    Port::outb(0xA1, 0);
    Port::outb(0x21, 0);

    idt_pointer.limit = 256 * sizeof(Entry) - 1;
    idt_pointer.base = (uint64_t)&idt_entries;
    asm volatile("lidt %0" ::"m"(idt_pointer));
    asm volatile("sti");
}

void Spark::Idt::set_gate(uint8_t vec, uintptr_t function, uint16_t selector, uint8_t flags) {
    idt_entries[vec].offset_low = function & 0xFFFF;
    idt_entries[vec].offset_mid = (function >> 16) & 0xFFFF;
    idt_entries[vec].offset_high = (function >> 32) & 0xFFFFFFFF;
    idt_entries[vec].selector = selector;
    idt_entries[vec].attrib = flags;
    idt_entries[vec].ist = 0;
    idt_entries[vec].zero = 0;
}