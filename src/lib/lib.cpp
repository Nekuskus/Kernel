#include "lib.hpp"

#include "ctype.hpp"

int sprintf(char *text, const char *format, ...) {
    va_list parameters;

    va_start(parameters, format);

    uint64_t written = 0;

    while (*format) {
        size_t maxrem = 2147483647 - written;

        if (format[0] != '%' || format[1] == '%') {
            if (format[0] == '%')
                format++;

            size_t amount = 0;

            for (; format[amount] && format[amount] != '%'; amount++)
                text[amount] = format[amount];

            if (maxrem < amount) {
                va_end(parameters);

                return -1;
            }

            text += amount;
            format += amount;
            written += amount;

            continue;
        }

        const char *format_begun_at = format++;

        switch (*format) {
            case 'c': {
                format++;
                auto c = (char)va_arg(parameters, int);

                if (!maxrem) {
                    va_end(parameters);

                    return -1;
                }

                *text = c;
                text += c != 0;
                written += c != 0;

                break;
            }

            case 's': {
                format++;
                auto str = va_arg(parameters, const char *);
                size_t len = strlen(str);

                if (maxrem < len) {
                    va_end(parameters);

                    return -1;
                }

                for (size_t i = 0; i < len; i++)
                    text[i] = str[i];

                text += len;
                written += len;

                break;
            }

            case 'i':
            case 'd': {
                auto item = va_arg(parameters, int);
                char str[32] = "";

                itoa(item, str, 10);

                size_t len = strlen(str);
                format++;

                if (maxrem < len) {
                    va_end(parameters);

                    return -1;
                }

                for (size_t i = 0; i < len; i++)
                    text[i] = str[i];

                text += len;
                written += len;
                break;
            }

            case 'x':
            case 'X': {
                auto item = va_arg(parameters, uint64_t);
                char str[32] = "";

                htoa(item, str, *format == 'X');

                size_t len = strlen(str);
                format++;

                if (maxrem < len) {
                    va_end(parameters);

                    return -1;
                }

                for (size_t i = 0; i < len; i++)
                    text[i] = str[i];

                text += len;
                written += len;
                break;
            }
            case 'p': {
                auto item = va_arg(parameters, void *);
                char str[32] = "";

                htoa((uintptr_t)item, str, false);

                size_t len = strlen(str);
                format++;

                if (maxrem < len) {
                    va_end(parameters);

                    return -1;
                }

                for (size_t i = 0; i < len; i++)
                    text[i] = str[i];

                text += len;
                written += len;

                break;
            }
            default: {
                format = format_begun_at;
                size_t len = strlen(format);

                if (maxrem < len) {
                    va_end(parameters);

                    return -1;
                }

                for (size_t i = 0; i < len; i++)
                    text[i] = format[i];

                written += len;
                format += len;

                break;
            }
        }
    }

    text[0] = '\0';

    va_end(parameters);

    return written;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && *s1 == *s2) {
        ++s1;
        ++s2;
        --n;
    }

    return !n ? 0 : s1 - s2;
}

size_t strlen(const char *chr) {
    size_t size = 0;

    while (*chr++)
        size++;

    return size;
}

size_t strnlen(const char *chr, size_t max_len) {
    size_t size = 0;

    while (size < max_len && chr[size])
        ++size;

    return size;
}

char *itoa(int value, char *result, int base) {
    if (base < 2 || base > 36) {
        *result = '\0';

        return result;
    }

    char *rc = result, *ptr, *low;
    rc = ptr = result;

    if (value < 0 && base == 10)
        *ptr++ = '-';

    low = ptr;

    do {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;

        *low++ = *ptr;
        *ptr-- = tmp;
    }

    return rc;
}

void htoa(uint64_t n, char *str, bool caps) {
    *str++ = '0';
    *str++ = 'x';

    int8_t zeros = 0;
    int64_t tmp = 0;

    for (int i = 60; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;

        if (tmp == 0 && zeros == 0)
            continue;

        zeros -= 1;
        *str++ = tmp >= 0xA ? tmp - 0xA + (caps ? 'A' : 'a') : tmp + '0';
    }

    tmp = n & 0xF;
    *str++ = tmp >= 0xA ? tmp - 0xA + (caps ? 'A' : 'a') : tmp + '0';
}

extern "C" void *memset(void *s, int c, size_t n) {
    auto p = (unsigned char *)s;
    auto fill = (unsigned char)c;

    while (n--)
        *p++ = fill;

    return s;
}

extern "C" void *memcpy(void *dest, const void *src, size_t len) {
    char *d = (char *)dest, *s = (char *)src;

    while (len--)
        *d++ = *s++;

    return dest;
}

// fuck off clang
extern "C" void atexit(...) {
}