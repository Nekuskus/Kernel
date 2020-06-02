#include <stdint.h>

#include <lib/lib.hpp>
#include <stivale.hpp>
#include <system/acpi/acpi.hpp>
#include <system/acpi/madt.hpp>
#include <system/cpu/apic.hpp>
#include <system/cpu/smp/smp.hpp>
#include <system/debugging.hpp>
#include <system/drivers/display/vbe.hpp>
#include <system/drivers/pci.hpp>
#include <system/drivers/storage/ahci.hpp>
#include <system/drivers/time.hpp>
#include <system/exceptions.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/pmm.hpp>
#include <system/mm/vmm.hpp>
#include <system/proc/task.hpp>

extern "C" void (*__CTOR_LIST__ [[gnu::visibility("hidden")]])();
extern "C" void (*__CTOR_END__ [[gnu::visibility("hidden")]])();
extern "C" void *stack_end;
extern "C" void init_gdt();

extern "C" [[gnu::section(".stivalehdr"), gnu::used]] Stivale::StivaleHeader header = {
    .stack = (uint64_t)&stack_end,
    .flags = 1,
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0,
    .entry_point = 0,
};

extern "C" void _start(void *stivale_ptr) {
    init_gdt();

    auto ctor = &__CTOR_LIST__;

    while (ctor != &__CTOR_END__)
        (*ctor++)();

    Stivale::Stivale *stivale = (Stivale::Stivale *)((uint64_t)stivale_ptr + virtual_kernel_base);

    Debug::print("~~~~ Welcome to Firework! ~~~~\n\r");

    Pmm::init((Stivale::StivaleMMapEntry *)stivale->memory_map_addr, stivale->memory_map_entries);
    Vmm::init();

    uint32_t *virtual_fb = (uint32_t *)(stivale->framebuffer_addr + virtual_kernel_base);

    Vmm::map_huge_pages(Vmm::get_ctx_kernel(), virtual_fb, (void *)stivale->framebuffer_addr, (stivale->framebuffer_width * stivale->framebuffer_pitch + 0x200000 - 1) / 0x200000, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);

    Graphics::ModeInfo mode_info = {
        .framebuffer = virtual_fb,
        .pitch = stivale->framebuffer_pitch,
        .width = stivale->framebuffer_width,
        .height = stivale->framebuffer_height,
        .bpp = stivale->framebuffer_bpp,
    };

    Graphics::init(mode_info);
    Idt::init();
    Exceptions::init();
    Acpi::init(stivale->rsdp);
    Hpet::init();
    Madt::init();
    Cpu::Apic::IoApic::init();
    Cpu::Apic::LocalApic::init();
    asm volatile("sti");
    Ahci::init();
    Tasking::init();
    Cpu::Smp::init();

    for (;;)
        asm volatile("hlt");
}