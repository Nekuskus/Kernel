#pragma once
#include <stdint.h>

namespace Stivale {
    struct [[gnu::packed]] StivaleHeader {
        uint64_t stack;
        uint16_t flags;
        uint16_t framebuffer_width;
        uint16_t framebuffer_height;
        uint16_t framebuffer_bpp;
        uint64_t entry_point;
    };

    struct [[gnu::packed]] Stivale {
        uint64_t cmdline;
        uint64_t memory_map_addr;
        uint64_t memory_map_entries;
        uint64_t framebuffer_addr;
        uint16_t framebuffer_pitch;
        uint16_t framebuffer_width;
        uint16_t framebuffer_height;
        uint16_t framebuffer_bpp;
        uint64_t rsdp;
        uint64_t module_count;
        uint64_t modules;
        uint64_t epoch;
        uint64_t flags;
    };
}  // namespace Stivale