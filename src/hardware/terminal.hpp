#pragma once
#include <stdint.h>

namespace Firework::FireworkKernel::Terminal {
    bool handle_special_characters(const char c);
    void write(const char* str, uint32_t foreground, uint32_t background);
    void write(const char* str, uint32_t foreground);
    void write(const char c, uint32_t foreground, uint32_t background);
    void write(const char c, uint32_t foreground);
    void write_line(const char* str, uint32_t foreground, uint32_t background);
    void write_line(const char* str, uint32_t foreground);
    void set_cursor(uint16_t x, uint16_t y);
};  // namespace Firework::FireworkKernel::Terminal