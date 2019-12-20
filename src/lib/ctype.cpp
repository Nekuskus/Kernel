#include <lib/ctype.hpp>

bool isalnum(int ch) {
    return isalpha(ch) || isdigit(ch);
}

bool isalpha(int ch) {
    return isupper(ch) || islower(ch);
}

bool isblank(int ch) {
    return ch == 0x09 || ch == 0x20;
}

bool iscntrl(int ch) {
    return (ch >= 0x00 && ch <= 0x1F) || ch == 0x7F;
}

bool isdigit(int ch) {
    return ch >= 0x30 && ch <= 0x39;
}

bool isgraph(int ch) {
    return ch >= 0x21 && ch <= 0x7A;
}

bool islower(int ch) {
    return ch >= 0x61 && ch <= 0x7A;
}

bool isprint(int ch) {
    return ch == 0x20 || isgraph(ch);
}

bool ispunct(int ch) {
    return (ch >= 0x21 && ch <= 0x2F) || (ch >= 0x3A && ch <= 0x40) || (ch >= 0x5B && ch <= 0x60) || (ch >= 0x7B && ch <= 0x7E);
}

bool isspace(int ch) {
    return ch == 0x20 || (ch >= 0x09 && ch <= 0x0D);
}

bool isupper(int ch) {
    return ch >= 0x41 && ch <= 0x5A;
}

bool isxdigit(int ch) {
    return (ch >= 0x41 && ch <= 0x46) || (ch >= 0x61 && ch <= 0x66) || isdigit(ch);
}

bool tolower(int ch) {
    return isupper(ch) ? ch ^ 0x20 : ch;
}

bool toupper(int ch) {
    return islower(ch) ? ch ^ 0x20 : ch;
}