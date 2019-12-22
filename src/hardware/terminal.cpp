#include <hardware/devices/vbe.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>

uint16_t x = 0, y = 0;

bool Firework::Terminal::handle_special_characters(const char c) {
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

void Firework::Terminal::set_cursor(uint16_t nx, uint16_t ny) {
    x = nx;
    y = ny;
}

void Firework::Terminal::write(const char* str, uint32_t foreground, uint32_t background) {
    uint32_t ln = strlen(str);

    for (uint32_t idx = 0; idx < ln; idx++)
        write(str[idx], foreground, background);
}

void Firework::Terminal::write(const char* str, uint32_t foreground) {
    uint32_t ln = strlen(str);

    for (uint32_t idx = 0; idx < ln; idx++)
        write(str[idx], foreground);
}

void Firework::Terminal::write(const char c, uint32_t foreground, uint32_t background) {
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

void Firework::Terminal::write(const char c, uint32_t foreground) {
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

void Firework::Terminal::write_line(const char* str, uint32_t foreground, uint32_t background) {
    write(str, foreground, background);
    write("\r\n", foreground, background);
}

void Firework::Terminal::write_line(const char* str, uint32_t foreground) {
    write(str, foreground);
    write("\r\n", foreground);
}