#pragma once
#include <hardware/idt.hpp>

namespace Firework {
    void panic(const char* message);
    void panic(const char* message, Firework::Idt::InterruptRegisters* registers);
}  // namespace Firework