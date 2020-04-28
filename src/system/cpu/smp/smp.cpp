#include "smp.hpp"

#include <cpuid.h>

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/acpi/madt.hpp>
#include <system/cpu/apic.hpp>
#include <system/cpu/cpu.hpp>
#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/pmm.hpp>
#include <system/mm/vmm.hpp>

extern "C" void* smp_entry;
extern "C" void* _trampoline_start;
extern "C" void* _trampoline_end;
extern "C" void* trampoline_stack;

extern "C" uint64_t stack_end;

extern "C" void load_tss(void* tss);

bool trampoline_booted = false;

bool Cpu::Smp::wait_for_boot() {
    for (int i = 0; i < 1000; i++) {
        Time::ksleep(1);

        if (trampoline_booted)
            return true;
    }

    return false;
}

void Cpu::Smp::boot_cpu(uint32_t lapic_id) {
    trampoline_stack = (void*)((uint64_t)Pmm::alloc(0x10000 / page_size) + virtual_physical_base);
    char debug[255] = "";

    if (!trampoline_stack) {
        sprintf(debug, "[SMP] Failed to allocate stack for CPU with lapic ID %d\n", lapic_id);
        Debug::print(debug);

        return;
    }

    auto cpu = new CpuState;
    cpu->id = lapic_id;
    cpu->thread = -1;
    cpu->process = -1;

    push(cpu);

    Tss* tss = new Tss;
    tss->rsp[0] = (uint64_t)trampoline_stack + 0x10000;

    load_tss(tss);

    cpu->tss = tss;

    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::TM_LEVEL | Apic::LocalApic::IcrFlags::LEVELASSERT | Apic::LocalApic::IcrFlags::DM_INIT);
    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uint64_t)&smp_entry >> 12) & 0xFF));

    if (!wait_for_boot()) Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uint64_t)&smp_entry >> 12) & 0xFF));

    if (wait_for_boot()) {
        cpu->booted = true;
        
        sprintf(debug, "[SMP] Sucessfully booted CPU with lapic ID %d\n", lapic_id);
        Debug::print(debug);
    } else {
        sprintf(debug, "[SMP] Failed to boot CPU with lapic ID %d\n", lapic_id);
        Debug::print(debug);
    }

    trampoline_booted = false;
}

void Cpu::Smp::set_booted() {
    trampoline_booted = true;
}

void Cpu::Smp::init() {
    Cpu::CpuState* current_cpu = Cpu::get_current_cpu();

    Tss* bsp_tss = new Tss;
    bsp_tss->rsp[0] = stack_end - 0x1000;

    load_tss(bsp_tss);

    current_cpu->tss = bsp_tss;

    uint64_t len = (uint64_t)&_trampoline_end - (uint64_t)&_trampoline_start;

    Vmm::map_pages(Vmm::get_current_context(), &_trampoline_start, &_trampoline_start, (len + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);
    memcpy(&_trampoline_start, (void*)(0x400000 + virtual_physical_base), len);

    uint32_t current_lapic = current_cpu->id;

    for (auto lapic : Madt::get_lapics())
        if (((lapic->flags & 1) || (lapic->flags & 2)) && lapic->id != current_lapic)
            Cpu::Smp::boot_cpu(lapic->id);

    Debug::print("[SMP] Finished setting up.\n");
}