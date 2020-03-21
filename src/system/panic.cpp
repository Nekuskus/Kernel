#include "panic.hpp"

#include <lib/lib.hpp>

#include "cpu/cpu.hpp"
#include "terminal.hpp"

void panic(const char* message) {
    char text[8192] = "";

    sprintf(text, ":( Kernel panic occurred on CPU %d,\n\rError message: %s", Cpu::get_current_cpu(), message);
    Terminal::write_line(text, 0xFFFFFF, 0xe50000);
    Cpu::halt_forever();
}