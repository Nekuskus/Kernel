#pragma once
#include <stddef.h>
#include <stdint.h>
constexpr size_t virtual_kernel_base = 0xFFFFFFFF80000000;
constexpr size_t virtual_physical_base = 0xFFFF800000000000;
constexpr size_t page_size = 0x1000;
constexpr size_t memory_base = 0x2000000;
constexpr size_t heap_base = 0x200000000;

enum MemoryFlags {
    MM_READ = 0x01,
    MM_WRITE = 0x02,
    MM_EXECUTE = 0x04,
    MM_USER = 0x08,
    MM_NO_CACHE = 0x10,
};

extern "C" void* malloc(size_t blocks);
extern "C" void* calloc(size_t blocks);
extern "C" void free(void* memory);
extern "C" void* memset(void* s, int c, size_t n);
extern "C" void* memcpy(void* dest, const void* src, size_t len);