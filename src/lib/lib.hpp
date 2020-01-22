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
extern "C" void* memset(void* s, int c, size_t n);
extern "C" void* memcpy(void* dest, const void* src, size_t len);