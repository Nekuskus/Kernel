#include <stdint.h>

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
#include <system/drivers/time.hpp>
#include <system/drivers/vbe.hpp>
#include <system/exceptions.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/pmm.hpp>
#include <system/mm/vmm.hpp>

static Graphics::ModeInfo mode_info{};

static size_t progress_bar = 0, progress_increase = 0;

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

extern "C" void (*__CTOR_LIST__ [[gnu::visibility("hidden")]])();
extern "C" void (*__CTOR_END__ [[gnu::visibility("hidden")]])();

extern "C" [[gnu::visibility("hidden")]] void __fw_ctors() {
    auto ctor = &__CTOR_LIST__;

    while (ctor != &__CTOR_END__)
        (*ctor++)();
}

extern "C" [[gnu::visibility("hidden")]] void __fw_dtors() {
    Debug::print("__fw_dtors was called\n\r");
}

extern "C" void kmain(void* mb_info_ptr, uint32_t multiboot_magic) {
    Debug::print("~~~~ Welcome to Firework! ~~~~\n\r");

    if (multiboot_magic == 0x2BADB002 && mb_info_ptr) {
        Multiboot::MultibootInfo* mb_info = (Multiboot::MultibootInfo*)((uint64_t)mb_info_ptr + virtual_kernel_base);
        Multiboot::MemoryMap* memory_map = (Multiboot::MemoryMap*)((uint64_t)mb_info->mmap_addr + virtual_kernel_base);

        Pmm::init(memory_map, mb_info->mmap_length / sizeof(*memory_map));
        Vmm::init();

        uintptr_t virtual_fb = mb_info->framebuffer_addr + virtual_kernel_base;

        Vmm::map_huge_pages(Vmm::get_ctx_kernel(), (void*)virtual_fb, (void*)mb_info->framebuffer_addr, (mb_info->framebuffer_width * mb_info->framebuffer_pitch + 0x200000 - 1) / 0x200000, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);

        mode_info = {
            .framebuffer = (uint32_t*)virtual_fb,
            .pitch = mb_info->framebuffer_pitch,
            .width = mb_info->framebuffer_width,
            .height = mb_info->framebuffer_height,
            .bpp = mb_info->framebuffer_bpp,
        };

        Graphics::init(mode_info);

        progress_increase = mode_info.width / 10;
        progress_bar += 10;

        Idt::init();

        progress();

        Exceptions::init();

        progress();

        Acpi::init();

        progress();

        Hpet::init();

        progress();

        Madt::init();

        progress();

        Cpu::Apic::IoApic::init();

        progress();

        Cpu::Apic::LocalApic::init();
        asm("sti");

        progress();

        Ahci::init();

        progress();

        Cpu::Multitasking::init();

        progress();

        Cpu::Smp::init();

        progress();
    } else {
        Debug::print("Unable to boot; Invalid Multiboot 1 magic or missing Multiboot info\n\r");

        return;
    }

    for (;;)
        asm volatile("hlt");
}

extern "C" void smp_kernel_main() {
    Idt::init();
    Exceptions::init();
    Cpu::Multitasking::init();
    Cpu::Smp::set_booted();

    asm("sti");

    for (;;)
        asm volatile("hlt");
}