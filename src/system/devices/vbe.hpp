#pragma once
#include <stddef.h>
#include <stdint.h>

namespace Graphics {
    struct ModeInfo {
        uint32_t* framebuffer;
        uint32_t pitch;
        uint32_t width;
        uint32_t height;
        uint8_t bpp;
    };

    void init(ModeInfo gfx_mode_info);
    void clear(uint32_t color);
    void write_text(const char c, uint16_t x, uint16_t y, uint32_t foreground);
    void write_text(const char* str, uint16_t x, uint16_t y, uint32_t foreground);
    void write_text(const char c, uint16_t x, uint16_t y, uint32_t foreground, uint32_t background);
    void write_text(const char* str, uint16_t x, uint16_t y, uint32_t foreground, uint32_t background);
    inline void set_pixel(uint16_t x, uint16_t y, uint32_t color);
    ModeInfo get_mode_info();
}  // namespace Graphics