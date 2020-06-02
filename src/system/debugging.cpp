#include "debugging.hpp"

#include "drivers/port.hpp"
#include "drivers/serial.hpp"

static auto debugging_serial = Serial(0x3F8, 115200);

void Debug::print(const char *text) {
    debugging_serial.write(text);
}