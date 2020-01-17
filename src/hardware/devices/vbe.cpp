#include "vbe.hpp"
#include <hardware/mm/mm.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>
#include "vgafont.hpp"

Firework::FireworkKernel::Graphics::ModeInfo mode_info;
bool is_working = false;

void Firework::FireworkKernel::Graphics::init(ModeInfo gfx_mode_info) {
    is_working = false;
    mode_info = gfx_mode_info;
    is_working = true;

    Firework::FireworkKernel::Graphics::clear(0x000000);
}

void Firework::FireworkKernel::Graphics::clear(uint32_t color) {
    if (!is_working)
        return;

    for (uint32_t y = 0; y < mode_info.height; y++)
        for (uint32_t x = 0; x < mode_info.width; x++)
            set_pixel(x, y, color);
}

void Firework::FireworkKernel::Graphics::write_text(const char c, uint16_t x, uint16_t y, uint32_t foreground) {
    if (!is_working || c == '\0')
        return;

    size_t font_off = c * 16;

    for (uint32_t ny = 0; ny < 16; ny++)
        for (uint32_t nx = 0; nx < 8; nx++)
            if (__font_bitmap__[font_off + ny] & (1 << (8 - nx)))
                set_pixel(x + nx, y + ny, foreground);
}

void Firework::FireworkKernel::Graphics::write_text(const char* str, uint16_t x, uint16_t y, uint32_t foreground) {
    if (!is_working)
        return;

    uint32_t ln = strlen(str);

    for (uint32_t idx = 0; idx < ln; idx++, x += 8)
        write_text(str[idx], x, y, foreground);
}

void Firework::FireworkKernel::Graphics::write_text(const char c, uint16_t x, uint16_t y, uint32_t foreground, uint32_t background) {
    if (!is_working || c == '\0' || c == '\n' || c == '\t' || c == '\r')
        return;

    size_t font_off = c * 16;

    for (uint32_t ny = 0; ny < 16; ny++)
        for (uint32_t nx = 0; nx < 8; nx++)
            set_pixel(x + nx, y + ny, __font_bitmap__[font_off + ny] & (1 << (8 - nx)) ? foreground : background);
}

void Firework::FireworkKernel::Graphics::write_text(const char* str, uint16_t x, uint16_t y, uint32_t foreground, uint32_t background) {
    if (!is_working)
        return;

    uint32_t ln = strlen(str);

    for (uint32_t idx = 0; idx < ln; idx++, x += 8)
        write_text(str[idx], x, y, foreground, background);
}

inline void Firework::FireworkKernel::Graphics::set_pixel(uint16_t x, uint16_t y, uint32_t color) {
    if (!is_working)
        return;

    mode_info.framebuffer[(x * 4 + y * mode_info.pitch) / (mode_info.bpp / 8)] = color;
}

Firework::FireworkKernel::Graphics::ModeInfo Firework::FireworkKernel::Graphics::get_mode_info() {
    return mode_info;
}