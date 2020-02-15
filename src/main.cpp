#include <stdint.h>

#include "firework_icon.hpp"
#include "multiboot.hpp"
#include "system/acpi/acpi.hpp"
#include "system/acpi/madt.hpp"
#include "system/cpu/apic.hpp"
#include "system/cpu/multitasking.hpp"
#include "system/cpu/smp/smp.hpp"
#include "system/debugging.hpp"
#include "system/drivers/pci.hpp"
#include "system/drivers/port.hpp"
#include "system/drivers/vbe.hpp"
#include "system/exceptions.hpp"
#include "system/idt.hpp"
#include "system/mm/mm.hpp"
#include "system/mm/pmm.hpp"
#include "system/mm/vmm.hpp"
#include "system/terminal.hpp"

extern "C" void* _init_array_begin;
extern "C" void* _init_array_end;

Graphics::ModeInfo mode_info{};

size_t progress_bar = 0, progress_increase = 0;

void progress() {
    if (progress_bar + progress_increase >= mode_info.width)
        progress_increase -= (progress_bar + progress_increase) - mode_info.width + 10;

    for (size_t new_progress = progress_bar + progress_increase; progress_bar < new_progress; progress_bar++) {
        Graphics::set_pixel(progress_bar, mode_info.height - 15, 0xFFFFFF);
        Graphics::set_pixel(progress_bar, mode_info.height - 14, 0xFFFFFF);
        Graphics::set_pixel(progress_bar, mode_info.height - 13, 0xFFFFFF);
        Graphics::set_pixel(progress_bar, mode_info.height - 12, 0xFFFFFF);
        Graphics::set_pixel(progress_bar, mode_info.height - 11, 0xFFFFFF);
        Graphics::set_pixel(progress_bar, mode_info.height - 10, 0xFFFFFF);
    }
}

extern "C" void kmain(void* mb_info_ptr, uint32_t multiboot_magic) {
    uintptr_t start = (uintptr_t)&_init_array_begin;
    uintptr_t end = (uintptr_t)&_init_array_end;

    for (uintptr_t ptr = start; ptr < end; ptr += 8)
        ((void (*)())(*(uintptr_t*)ptr))();

    Debug::init();
    Debug::print("~~~~ Welcome to Firework! ~~~~\n");

    if (multiboot_magic == 0x2BADB002 && mb_info_ptr) {
        Multiboot::MultibootInfo* mb_info = (Multiboot::MultibootInfo*)((uint64_t)mb_info_ptr + virtual_kernel_base);
        Multiboot::MemoryMap* memory_map = (Multiboot::MemoryMap*)((uint64_t)mb_info->mmap_addr + virtual_kernel_base);

        Pmm::init(memory_map, mb_info->mmap_length / sizeof(*memory_map));
        Vmm::init();

        uintptr_t virtual_fb = mb_info->framebuffer_addr + virtual_kernel_base;

        Vmm::map_huge_pages(Vmm::get_ctx_kernel(), (void*)virtual_fb, (void*)mb_info->framebuffer_addr, (mb_info->framebuffer_width * mb_info->framebuffer_pitch + huge_page_size - 1) / huge_page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

        mode_info = {
            .framebuffer = (uint32_t*)virtual_fb,
            .pitch = mb_info->framebuffer_pitch,
            .width = mb_info->framebuffer_width,
            .height = mb_info->framebuffer_height,
            .bpp = mb_info->framebuffer_bpp,
        };

        Graphics::init(mode_info);

        progress_increase = mode_info.width / 8;
        progress_bar += 10;

        progress();

        Idt::init();
        progress();
        Exceptions::init();
        progress();

        uint16_t start_of_img_x = mode_info.width / 2 - 128, start_of_img_y = mode_info.height / 2 - 128;

        for (size_t y = 0; y < 256; y++) {
            for (size_t x = 0; x < 256; x++) {
                uint8_t* rgb = (uint8_t*)firework_icon + (256 * y + x) * 3;
                union {
                    struct {
                        char r;
                        char g;
                        char b;
                        char a;
                    } color;
                    uint32_t int_color;
                } color;

                color.color.r = rgb[0];
                color.color.g = rgb[1];
                color.color.b = rgb[2];

                Graphics::set_pixel(start_of_img_x + x, start_of_img_y + y, color.int_color);
            }
        }

        Acpi::init();
        progress();
        Madt::init();
        progress();
        Cpu::Apic::LocalApic::init();
        progress();
        Cpu::Apic::IoApic::init();
        asm("sti");
        progress();
        Cpu::Smp::init();
        progress();
        Pci::init();
        progress();
        //Cpu::Multitasking::init();
    } else {
        Debug::print("!! Unable to boot; Invalid multiboot1 magic or missing multiboot info !!\n");

        return;
    }

    while (1)
        asm volatile("hlt");
}

extern "C" void smp_kernel_main() {
    Idt::init();
    Cpu::Smp::set_booted();

    while (1)
        asm volatile("hlt");
}