#include <hardware/cpu/smp/smp.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/pmm.hpp>
#include <hardware/mm/vmm.hpp>
#include <lib/lib.hpp>
#include <hardware/acpi/apic.hpp>
#include <hardware/terminal.hpp>

bool trampoline_booted = false;
extern "C" void* smp_entry;
extern "C" void* _trampoline_start;
extern "C" void* _trampoline_end;
extern "C" void* trampoline_stack;

bool Spark::Cpu::Smp::wait_for_boot() {
    for (uint64_t timeout = 100; timeout > 0; timeout--) {
        if (trampoline_booted)
            return true;

        for (uint64_t i = 100000; i > 0; i--)
            ;
    }

    return false;
}

void Spark::Cpu::Smp::init() {
    uint64_t len = (uintptr_t)&_trampoline_end - (uintptr_t)&_trampoline_start;

    Vmm::map_pages(Vmm::get_current_context(), &_trampoline_start, &_trampoline_start, (len + page_size - 1) / page_size, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);
    memcpy(&_trampoline_start, (void*)(0x400000 + virtual_physical_base), len);
}

void Spark::Cpu::Smp::boot_cpu(uint32_t lapic_id) {
    trampoline_stack = (void*)((uintptr_t)Pmm::alloc(0x10000 / page_size) + virtual_physical_base);
    char debug[255] = "";

    if (trampoline_stack == nullptr) {
        sprintf(debug, "[SMP] Failed to allocate stack for CPU with lapic ID %d", lapic_id);
        Terminal::write_line(debug, 0xFFFFFF);
        return;
    }

    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::TM_LEVEL | Apic::LocalApic::IcrFlags::LEVELASSERT | Apic::LocalApic::IcrFlags::DM_INIT);
    Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uintptr_t)&smp_entry >> 12) & 0xFF));

    if (!wait_for_boot()) Apic::LocalApic::send_ipi(lapic_id, Apic::LocalApic::IcrFlags::DM_SIPI | (((uintptr_t)&smp_entry >> 12) & 0xFF));

    if (wait_for_boot()) {
        sprintf(debug, "[SMP] Sucessfully booted CPU with lapic ID %d", lapic_id);
        Terminal::write_line(debug, 0xFFFFFF);
    } else {
        sprintf(debug, "[SMP] Failed to boot CPU with lapic ID %d", lapic_id);
        Terminal::write_line(debug, 0xFFFFFF);
    }

    trampoline_booted = false;
}

void Spark::Cpu::Smp::set_booted() {
    trampoline_booted = true;
}