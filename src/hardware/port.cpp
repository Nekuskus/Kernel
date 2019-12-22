#include <hardware/port.hpp>

void Firework::Port::outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1"
                 :
                 : "a"(value), "Nd"(port));
}

uint8_t Firework::Port::inb(uint16_t port) {
    uint8_t ret;

    asm volatile("inb %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));

    return ret;
}

void Firework::Port::outw(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1"
                 :
                 : "a"(value), "Nd"(port));
}

uint16_t Firework::Port::inw(uint16_t port) {
    uint16_t ret;

    asm volatile("inw %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));

    return ret;
}

void Firework::Port::outd(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1"
                 :
                 : "a"(value), "Nd"(port));
}

uint32_t Firework::Port::ind(uint16_t port) {
    uint32_t ret;

    asm volatile("inl %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));

    return ret;
}

void Firework::Port::wait() {
    outb(0x80, 0);
}