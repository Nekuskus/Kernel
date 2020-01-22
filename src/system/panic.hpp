#pragma once
#include "idt.hpp"

void panic(const char* message);
void panic(const char* message, Idt::InterruptRegisters* registers);