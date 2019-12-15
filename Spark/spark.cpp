#include <stdint.h>
#include <hardware/acpi/acpi.hpp>
#include <hardware/acpi/apic.hpp>
#include <hardware/cpu/cpu.hpp>
#include <hardware/cpu/smp/smp.hpp>
#include <hardware/devices/pci.hpp>
#include <hardware/devices/vbe.hpp>
#include <hardware/idt.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/pmm.hpp>
#include <hardware/mm/vmm.hpp>
#include <hardware/terminal.hpp>
#include <lib/lib.hpp>
#include <multiboot.hpp>
#include <hardware/exceptions.hpp>

namespace Spark {
    extern "C" void kernel_main(void* mb_info_ptr, uint32_t multiboot_magic) {
        if (multiboot_magic == 0x2BADB002 && mb_info_ptr) {
            Multiboot::Info* mb_info = (Multiboot::Info*)((uint64_t)mb_info_ptr + virtual_kernel_base);
            Multiboot::MemoryMap* memory_map = (Multiboot::MemoryMap*)(uint64_t)mb_info->mmap_addr;
            Pmm::init(memory_map, mb_info->mmap_length / sizeof(*memory_map));
            Vmm::init();
            void* virtual_fb = (void*)(mb_info->framebuffer_addr + virtual_kernel_base);

            if (Vmm::map_pages(Vmm::get_current_context(), virtual_fb, (void*)mb_info->framebuffer_addr, (mb_info->framebuffer_width * mb_info->framebuffer_pitch + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE)) {
                Graphics::ModeInfo mode_info = {
                    .backbuffer = (uint32_t*)malloc(mb_info->framebuffer_width * mb_info->framebuffer_pitch),
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
                Apic::init();
                Pci::init();
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
}  // namespace Spark