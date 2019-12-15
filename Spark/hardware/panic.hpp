#pragma once
#include <hardware/idt.hpp>

namespace Spark {
    void panic(const char* message);
    void panic(const char* message, Spark::Idt::InterruptRegisters* registers);
}  // namespace Spark