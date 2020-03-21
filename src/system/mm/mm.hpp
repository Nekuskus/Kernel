#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr size_t virtual_kernel_base = 0xFFFFFFFF80000000;
constexpr size_t virtual_physical_base = 0xFFFF800000000000;
constexpr size_t page_size = 0x1000;
constexpr size_t huge_page_size = 0x200000;
constexpr size_t memory_base = 0x2000000;
constexpr size_t heap_base = 0xffffffffd0000000;

enum MemoryFlags {
    MM_READ = 0x01,
    MM_WRITE = 0x02,
    MM_EXECUTE = 0x04,
    MM_USER = 0x08,
    MM_NO_CACHE = 0x10,
};

struct HeapHeader {
    uint64_t size;
    uint64_t pages;
};

extern "C" void* malloc(size_t bytes);
extern "C" void* calloc(size_t bytes, size_t elem);
extern "C" void free(void* ptr);