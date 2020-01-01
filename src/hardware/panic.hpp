#pragma once
#include <hardware/idt.hpp>

namespace Firework::FireworkKernel {
    void panic(const char* message);
    void panic(const char* message, Firework::FireworkKernel::Idt::InterruptRegisters* registers);
}  // namespace Firework