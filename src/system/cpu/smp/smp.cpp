#include "smp.hpp"

#include <cpuid.h>

#include <lib/lib.hpp>
#include <system/acpi/madt.hpp>
#include <system/cpu/apic.hpp>
#include <system/cpu/cpu.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/pmm.hpp>
#include <system/mm/vmm.hpp>

extern "C" void* smp_entry;
extern "C" void* _trampoline_start;
extern "C" void* _trampoline_end;
extern "C" void* trampoline_stack;

bool trampoline_booted = false;

bool Cpu::Smp::wait_for_boot() {
    for (uint64_t timeout = 100; timeout > 0; timeout--) {
        if (trampoline_booted)
            return true;

        for (uint64_t i = 100000; i > 0; i--)
            ;
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

    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::TM_LEVEL | Apic::LocalApic::IcrFlags::LEVELASSERT | Apic::LocalApic::IcrFlags::DM_INIT);
    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uint64_t)&smp_entry >> 12) & 0xFF));

    if (!wait_for_boot()) Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uintptr_t)&smp_entry >> 12) & 0xFF));

    if (wait_for_boot()) {
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
    uint64_t len = (uint64_t)&_trampoline_end - (uint64_t)&_trampoline_start;

    Vmm::map_pages(Vmm::get_current_context(), &_trampoline_start, &_trampoline_start, (len + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);
    memcpy(&_trampoline_start, (void*)(0x400000 + virtual_physical_base), len);

    uint32_t current_lapic = Cpu::get_current_cpu();

    for (auto& lapic : Madt::get_lapics())
        if ((lapic->flags & 1) && lapic->id != current_lapic)
            Cpu::Smp::boot_cpu(lapic->id);

    Debug::print("[SMP] Finished setting up.\n");
}