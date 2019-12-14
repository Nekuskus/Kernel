#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

int sprintf(char* text, const char* format, ...);
int strncmp(const char* str1, const char* str2, size_t len);
size_t strlen(const char*);
size_t strnlen(const char* chr, size_t max_len);
char* itoa(int value, char* result, int base);
void htoa(uint64_t n, char* str, bool caps);

template <class InputIt, class OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
    for (; first != last; first++)
        *d_first++ = *first++;
    return d_first;
}

template <class InputIt, class OutputIt, class UnaryPredicate>
OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred) {
    for (; first != last; first++)
        if (pred(*first))
            *d_first++ = *first;
    return d_first;
}