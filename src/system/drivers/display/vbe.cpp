#include "vbe.hpp"

#include <system/debugging.hpp>

#include "vgafont.hpp"

static Graphics::ModeInfo mode_info;
static bool is_working = false;

void Graphics::init(ModeInfo gfx_mode_info) {
    is_working = false;
    mode_info = gfx_mode_info;

    clear(0x000000);
    is_working = true;
    Debug::print("[VBE] Initialized display.\n\r");
}

void Graphics::clear(uint32_t color) {
    if (!is_working)
        return;

    for (uint32_t y = 0; y < mode_info.height; y++)
        for (uint32_t x = 0; x < mode_info.width; x++)
            set_pixel(x, y, color);
}

void Graphics::write_text(const char c, uint16_t x, uint16_t y, uint32_t foreground) {
    if (!is_working || c == '\0' || c == '\n' || c == '\t' || c == '\r')
        return;

    size_t font_off = c * 16;

    for (uint32_t ny = 0; ny < 16; ny++)
        for (uint32_t nx = 0; nx < 8; nx++)
            if (__font_bitmap__[font_off + ny] & (1 << (8 - nx)))
                set_pixel(x + nx, y + ny, foreground);
}

void Graphics::write_text(const char *str, uint16_t x, uint16_t y, uint32_t foreground) {
    if (!is_working)
        return;

    for (; *str; str++, x += 8)
        write_text(*str, x, y, foreground);
}

void Graphics::write_text(const char c, uint16_t x, uint16_t y, uint32_t foreground, uint32_t background) {
    if (!is_working || c == '\0' || c == '\n' || c == '\t' || c == '\r')
        return;

    size_t font_off = c * 16;

    for (uint32_t ny = 0; ny < 16; ny++)
        for (uint32_t nx = 0; nx < 8; nx++)
            set_pixel(x + nx, y + ny, __font_bitmap__[font_off + ny] & (1 << (8 - nx)) ? foreground : background);
}

void Graphics::write_text(const char *str, uint16_t x, uint16_t y, uint32_t foreground, uint32_t background) {
    if (!is_working)
        return;

    for (; *str; str++, x += 8)
        write_text(*str, x, y, foreground, background);
}

void Graphics::set_pixel(uint16_t x, uint16_t y, uint32_t color) {
    if (!is_working)
        return;

    mode_info.framebuffer[(x * 4 + y * mode_info.pitch) / (mode_info.bpp / 8)] = color;
}

Graphics::ModeInfo &Graphics::get_mode_info() {
    return mode_info;
}