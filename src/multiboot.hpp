#pragma once
#include <stdint.h>

namespace Firework::FireworkKernel::Multiboot {
    enum MemoryState {
        AVAILABLE = 1,
        RESERVED = 2,
        ACPI_RECLAIMABLE = 3,
        NVS = 4,
        BADRAM = 5,
    };

    struct Info {
        uint32_t flags;
        uint32_t mem_lower;
        uint32_t mem_upper;
        uint32_t boot_device;
        uint32_t cmdline;
        uint32_t mods_count;
        uint32_t mods_addr;
        union {
            struct AoutSymbolTable {
                uint32_t tabsize;
                uint32_t strsize;
                uint32_t addr;
                uint32_t reserved;
            } aout_sym;
            struct ElfSectionHeaderTable {
                uint32_t num;
                uint32_t size;
                uint32_t addr;
                uint32_t shndx;
            } elf_sec;
        } u;
        uint32_t mmap_length;
        uint32_t mmap_addr;
        uint32_t drives_length;
        uint32_t drives_addr;
        uint32_t config_table;
        uint32_t boot_loader_name;
        uint32_t apm_table;
        uint32_t vbe_control_info;
        uint32_t vbe_mode_info;
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;
        uint64_t framebuffer_addr;
        uint32_t framebuffer_pitch;
        uint32_t framebuffer_width;
        uint32_t framebuffer_height;
        uint8_t framebuffer_bpp;
        uint8_t framebuffer_type;
        union {
            struct
            {
                uint32_t framebuffer_palette_addr;
                uint16_t framebuffer_palette_num_colors;
            };
            struct
            {
                uint8_t framebuffer_red_field_position;
                uint8_t framebuffer_red_mask_size;
                uint8_t framebuffer_green_field_position;
                uint8_t framebuffer_green_mask_size;
                uint8_t framebuffer_blue_field_position;
                uint8_t framebuffer_blue_mask_size;
            };
        };
    };

    struct Color {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct [[gnu::packed]] MemoryMap {
        uint32_t size;
        uint64_t addr;
        uint64_t len;
        uint32_t type;
    };

    struct Module {
        uint32_t mod_start;
        uint32_t mod_end;
        uint32_t cmdline;
        uint32_t pad;
    };

    struct ApmInfo {
        uint16_t version;
        uint16_t cseg;
        uint32_t offset;
        uint16_t cseg_16;
        uint16_t dseg;
        uint16_t flags;
        uint16_t cseg_len;
        uint16_t cseg_16_len;
        uint16_t dseg_len;
    };
}  // namespace Firework::FireworkKernel::Multiboot