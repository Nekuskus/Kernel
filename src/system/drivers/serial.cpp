#include "serial.hpp"

#include "port.hpp"

Serial::Serial(uint16_t com_port, uint32_t baud_rate)
    : lock() {
    base = com_port;

    Port::outb(base + 3, 0x03);
    Port::outb(base + 1, 0x00);
    set_baud_rate(baud_rate);
}

Serial::~Serial() {
    Port::outb(base + 1, 0x00);
}

void Serial::set_baud_rate(uint32_t baud_rate) {
    uint8_t cmd = Port::inb(base + 3) | (1ull << 7);
    Port::outb(base + 3, cmd);

    uint16_t divisor = 115200 / baud_rate;

    Port::outb(base, (divisor >> 8) & 0xFF);
    Port::outb(base + 1, divisor & 0xFF);

    cmd = Port::inb(base + 3) & ~(1ull << 7);

    Port::outb(base + 3, cmd);
}

bool Serial::transmit_empty() {
    return Port::inb(base + 5) & 0x20;
}

bool Serial::received() {
    return Port::inb(base + 5) & 1;
}

char Serial::read() {
    while (!received())
        ;

    return Port::inb(base);
}

void Serial::write(const char a) {
    while (!transmit_empty())
        ;

    Port::outb(base, a);
}

void Serial::write(const char *a) {
    lock.lock();

    while (*a)
        write(*a++);

    lock.release();
}