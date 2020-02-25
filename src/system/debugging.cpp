#include "debugging.hpp"

#include "drivers/port.hpp"
#include "drivers/serial.hpp"

static Serial debugging_serial = Serial(0x3F8, 115200);

void Debug::print(const char* text) {
    debugging_serial.write(text);

    while (*text) {
        Port::outb(0x504, *text);
        text++;
    }
}