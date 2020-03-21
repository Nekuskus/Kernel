#include "gdt.hpp"

#include <lib/lib.hpp>

#include "cpu/cpu.hpp"
#include "debugging.hpp"
#include "mm/mm.hpp"

extern "C" uint64_t stack_end;

void Gdt::init() {
    Tss* tss = new Tss;
    tss->rsp[0] = stack_end;

    uint32_t* gdt = (uint32_t*)malloc(sizeof(uint64_t) * 6);
    gdt[0] = gdt[1] = gdt[2] = gdt[4] = gdt[6] = gdt[11] = 0;
    gdt[3] = 0xA09800;
    gdt[5] = 0xA0F800;
    gdt[7] = 0xA0F200;
    gdt[8] = (sizeof(Tss) & 0xFFFF) | (((uint64_t)tss & 0xFFFF) << 16);
    gdt[9] = (((uint64_t)tss >> 16) & 0xFF) | 0x8900 | (sizeof(Tss) & 0x000F0000) | ((uint64_t)tss & 0xFF000000);
    gdt[10] = (uint64_t)tss >> 32;

    Gdtr gdtr{
        .len = 6 * 8 - 1,
        .ptr = (uint64_t)gdt,
    };

    asm volatile("lgdt %0" ::"m"(gdtr));
    asm volatile("ltr %0" ::"r"((uint16_t)0x20));

    char debug[256] = "";
    sprintf(debug, "[GDT] Successfully initialized GDT on CPU #%d.\n", Cpu::get_current_cpu());
    Debug::print(debug);
}