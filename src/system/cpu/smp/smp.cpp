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

extern "C" void* trampoline;
extern "C" void* trampoline_end;
extern "C" void* stack_end;

extern "C" void load_tss(uint64_t tss);
extern "C" void prepare_trampoline(uint64_t page_table, uint64_t smp_entry, uint64_t trampoline_stack);
extern "C" void smp_kernel_main();

static bool trampoline_booted = false;

inline bool wait_for_boot() {
    for (int i = 0; i < 1000; i++)
        if (Time::ksleep(1); trampoline_booted)
            return true;

    return false;
}

void Cpu::Smp::boot_cpu(uint64_t kernel_pml4, uint32_t lapic_id) {
    void* trampoline_stack = calloc(0x1000, 1);
    char debug[255] = "";

    if (!trampoline_stack) {
        sprintf(debug, "[SMP] Failed to allocate stack for CPU with lapic ID %d\n\r", lapic_id);
        Debug::print(debug);

        return;
    }

    uint64_t trampoline_stack_end = (uint64_t)trampoline_stack + 0x1000;

    auto cpu = new CpuState;
    cpu->id = lapic_id;

    push(cpu);

    Tss* tss = new Tss;
    tss->rsp[0] = trampoline_stack_end;

    load_tss((uint64_t)tss);

    cpu->tss = tss;

    prepare_trampoline(kernel_pml4, (uint64_t)smp_kernel_main, trampoline_stack_end);

    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::TM_LEVEL | Apic::LocalApic::IcrFlags::LEVELASSERT | Apic::LocalApic::IcrFlags::DM_INIT);
    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uint64_t)0x1000 >> 12) & 0xFF));

    if (!wait_for_boot())
        Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uint64_t)0x1000 >> 12) & 0xFF));

    if (wait_for_boot()) {
        cpu->booted = true;

        sprintf(debug, "[SMP] Booted CPU with lapic ID #%d\n\r", lapic_id);
        Debug::print(debug);
    } else {
        sprintf(debug, "[SMP] Failed to boot CPU with lapic ID #%d\n\r", lapic_id);
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
    bsp_tss->rsp[0] = (uint64_t)&stack_end;

    load_tss((uint64_t)bsp_tss);

    current_cpu->tss = bsp_tss;

    auto len = (uint64_t)&trampoline_end - (uint64_t)&trampoline;
    auto kernel_pml4 = Vmm::get_ctx_kernel();

    Vmm::map_pages(kernel_pml4, (void*)0x1000, (void*)0x1000, (len + 0x1000 - 1) / 0x1000, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);
    memcpy((void*)0x1000, &trampoline, len);

    Vmm::map_pages(kernel_pml4, (void*)0x510, (void*)0x510, 1, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);

    for (auto lapic : Madt::get_lapics())
        if (((lapic->flags & 1) || (lapic->flags & 2)) && lapic->id != current_cpu->id)
            Cpu::Smp::boot_cpu((uint64_t)kernel_pml4, lapic->id);

    Debug::print("[SMP] Finished setting up.\n\r");
}