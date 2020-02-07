#include <stdint.h>

#include <multiboot.hpp>
#include <system/acpi/acpi.hpp>
#include <system/acpi/apic.hpp>
#include <system/acpi/madt.hpp>
#include <system/cpu/multitasking.hpp>
#include <system/cpu/smp/smp.hpp>
#include <system/devices/pci.hpp>
#include <system/devices/vbe.hpp>
#include <system/exceptions.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/pmm.hpp>
#include <system/mm/vmm.hpp>

extern "C" void kmain(void* mb_info_ptr, uint32_t multiboot_magic) {
    if (multiboot_magic == 0x2BADB002 && mb_info_ptr) {
        Multiboot::MultibootInfo* mb_info = (Multiboot::MultibootInfo*)((uint64_t)mb_info_ptr + virtual_kernel_base);
        Multiboot::MemoryMap* memory_map = (Multiboot::MemoryMap*)((uint64_t)mb_info->mmap_addr + virtual_kernel_base);

        Pmm::init(memory_map, mb_info->mmap_length / sizeof(*memory_map));
        Vmm::init();

        uintptr_t virtual_fb = mb_info->framebuffer_addr + virtual_kernel_base;

        if (Vmm::map_pages(Vmm::get_current_context(), (void*)virtual_fb, (void*)mb_info->framebuffer_addr, (mb_info->framebuffer_width * mb_info->framebuffer_pitch + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE)) {
            Graphics::ModeInfo mode_info = {
                .framebuffer = (uint32_t*)virtual_fb,
                .pitch = mb_info->framebuffer_pitch,
                .width = mb_info->framebuffer_width,
                .height = mb_info->framebuffer_height,
                .bpp = mb_info->framebuffer_bpp,
            };

            Graphics::init(mode_info);
            Idt::init();
            Exceptions::init();
            Acpi::init();
            Madt::init();
            Apic::IoApic::init();
            Apic::LocalApic::init();
            Cpu::Smp::init();
            Pci::init();
            Cpu::Multitasking::init();
        } else
            return;
    } else
        return;

    while (1)
        asm volatile("hlt");
}

extern "C" void smp_kernel_main() {
    Idt::init();
    Cpu::Smp::set_booted();

    while (1)
        asm volatile("hlt");
}