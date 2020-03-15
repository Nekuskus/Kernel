#include <stdint.h>

#include <firework_icon.hpp>
#include <lib/lib.hpp>
#include <multiboot.hpp>
#include <system/acpi/acpi.hpp>
#include <system/acpi/madt.hpp>
#include <system/cpu/apic.hpp>
#include <system/cpu/multitasking.hpp>
#include <system/cpu/smp/smp.hpp>
#include <system/debugging.hpp>
#include <system/drivers/ahci.hpp>
#include <system/drivers/pci.hpp>
#include <system/drivers/vbe.hpp>
#include <system/exceptions.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/pmm.hpp>
#include <system/mm/vmm.hpp>
#include <system/drivers/timer.hpp>

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

size_t last_len = 0;

void show_progress(char* text) {
    size_t len = strlen(text), actual_len = len;

    bool created_new_text = false;

    if (last_len > len) {
        actual_len = last_len;

        created_new_text = true;
        char* new_text = new char[last_len];

        memcpy(new_text + (last_len - len) / 2, text, len);

        text = new_text;

        for (size_t i = 0; i < (last_len - len) / 2; i++)
            text[i] = text[(last_len - len) / 2 + len + i] = ' ';

        text[last_len] = '\0';
    }

    Graphics::write_text(text, mode_info.width / 2 - actual_len * 4, mode_info.height / 2 + 148, 0xFFFFFF, 0x000000);

    if (created_new_text)
        delete text;

    last_len = len;
}

extern "C" void (*__CTOR_LIST__ [[gnu::visibility("hidden")]])();
extern "C" void (*__CTOR_END__ [[gnu::visibility("hidden")]])();

extern "C" [[gnu::visibility("hidden")]] void __fw_ctors() {
    auto ctor = &__CTOR_LIST__;

    while (ctor != &__CTOR_END__)
        (*ctor++)();
}

extern "C" [[gnu::visibility("hidden")]] void __fw_dtors() {
    Debug::print("__fw_dtors was called");
}

extern "C" void kmain(void* mb_info_ptr, uint32_t multiboot_magic) {
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

        progress_increase = mode_info.width / 11;
        progress_bar += 10;

        show_progress((char*)"Initializing display");
        progress();
        show_progress((char*)"Initializing IDT");
        Idt::init();
        progress();
        show_progress((char*)"Initializing exception handlers");
        Exceptions::init();
        progress();

        uint16_t start_of_img_x = mode_info.width / 2 - 128, start_of_img_y = mode_info.height / 2 - 128;
        const char* rgb = firework_icon;

        union {
            struct {
                char r;
                char g;
                char b;
                char a;
            } color;
            uint32_t int_color;
        } color;

        for (size_t y = 0; y < 256; y++)
            for (size_t x = 0; x < 256; x++) {
                color.color.r = rgb[0];
                color.color.g = rgb[1];
                color.color.b = rgb[2];

                Graphics::set_pixel(start_of_img_x + x, start_of_img_y + y, color.int_color);
                rgb = (const char*)((uint64_t)firework_icon + (256 * y + x) * 3);
            }

        show_progress((char*)"Initializing ACPI");
        Acpi::init();
        progress();
        show_progress((char*)"Initializing MADT");
        Madt::init();
        progress();
        show_progress((char*)"Initializing LAPIC");
        Cpu::Apic::LocalApic::init();
        progress();
        show_progress((char*)"Initializing IOAPIC");
        Cpu::Apic::IoApic::init();
        show_progress((char*)"Enabling interrupts");
        asm("sti");
        progress();
        show_progress((char*)"Initializing symmetric multiprocessing");
        Cpu::Smp::init();
        progress();
        show_progress((char*)"Initializing AHCI");
        Ahci::init();
        progress();
        show_progress((char*)"Initializing HPET");
        Hpet::init();
        progress();
        show_progress((char*)"Initializing task scheduler");
        Cpu::Multitasking::init();
        progress();
        show_progress((char*)"");
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