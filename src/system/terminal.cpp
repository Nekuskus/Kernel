#include "terminal.hpp"

#include <lib/lib.hpp>

#include "drivers/vbe.hpp"

static uint16_t x = 0, y = 0;

bool Terminal::handle_special_characters(const char c) {
    bool ret = false;

    switch (c) {
        case '\n': {
            y++;
            ret = true;

            break;
        }
        case '\t': {
            x += 4;
            ret = true;

            break;
        }
        case '\r': {
            x = 0;
            ret = true;

            break;
        }
    }

    set_cursor(x, y);

    return ret;
}

void Terminal::set_cursor(uint16_t nx, uint16_t ny) {
    x = nx;
    y = ny;
}

void Terminal::write(const char* str, uint32_t foreground, uint32_t background) {
    while (*str) {
        write(*str, foreground, background);
        str++;
    }
}

void Terminal::write(const char* str, uint32_t foreground) {
    while (*str) {
        write(*str, foreground);
        str++;
    }
}

void Terminal::write(const char c, uint32_t foreground, uint32_t background) {
    if (handle_special_characters(c))
        return;

    Graphics::ModeInfo mode_info = Graphics::get_mode_info();

    if (x * 8 >= mode_info.width) {
        x = 0;
        y++;
    }

    if (y * 16 >= mode_info.height) {
        x = 0;
        y = mode_info.height / 16 - 1;
    }

    Graphics::write_text(c, x * 8, y * 16, foreground, background);
    set_cursor(++x, y);
}

void Terminal::write(const char c, uint32_t foreground) {
    if (handle_special_characters(c))
        return;

    Graphics::ModeInfo mode_info = Graphics::get_mode_info();

    if (x * 8 >= mode_info.width) {
        x = 0;
        y++;
    }

    if (y * 16 >= mode_info.height) {
        x = 0;
        y = mode_info.height / 16 - 1;
    }

    Graphics::write_text(c, x * 8, y * 16, foreground);
    set_cursor(++x, y);
}

void Terminal::write_line(const char* str, uint32_t foreground, uint32_t background) {
    write(str, foreground, background);
    write("\r\n", foreground, background);
}

void Terminal::write_line(const char* str, uint32_t foreground) {
    write(str, foreground);
    write("\r\n", foreground);
}