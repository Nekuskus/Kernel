#include <cpuid.h>
#include <hardware/acpi/acpi.hpp>
#include <hardware/acpi/apic.hpp>
#include <hardware/cpu/smp/smp.hpp>
#include <hardware/mm/mm.hpp>
#include <hardware/mm/vmm.hpp>
#include <hardware/msr.hpp>

Firework::FireworkKernel::Acpi::MadtHeader* madt;
uint64_t lapic_base;

uint32_t Firework::FireworkKernel::Apic::LocalApic::read(uint32_t reg) {
    uint32_t* value = (uint32_t*)(lapic_base + reg);

    return *value;
}

void Firework::FireworkKernel::Apic::LocalApic::write(uint32_t reg, uint32_t data) {
    uint32_t* value = (uint32_t*)(lapic_base + reg);
    *value = data;
}

void Firework::FireworkKernel::Apic::LocalApic::init() {
    uint64_t apic_msr_base = Msr::read(apic_base);
    lapic_base = (apic_base & 0xFFFFFFFFFFFFF000) + virtual_kernel_base;
    apic_msr_base |= 1 << 11;

    Vmm::map_pages(Vmm::get_current_context(), lapic_base, apic_msr_base & 0xFFFFFFFFFFFFF000, 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);
    Msr::write(apic_base, apic_msr_base);
}

void Firework::FireworkKernel::Apic::LocalApic::send_ipi(uint32_t target, uint32_t flags) {
    write(icr_high, target << 24);
    write(icr_low, flags);

    while (read(icr_low) & IcrFlags::PENDING)
        ;
}

void Firework::FireworkKernel::Apic::init() {
    madt = (Firework::FireworkKernel::Acpi::MadtHeader*)Acpi::get_table("APIC");
    size_t table_size = madt->header.length - sizeof(Acpi::MadtHeader);
    uint64_t list = (uint64_t)madt + sizeof(Acpi::MadtHeader), offset = 0;

    LocalApic::init();
    Cpu::Smp::init();

    uint32_t a, b, c, d;
    __cpuid(1, a, b, c, d);

    while (offset < table_size) {
        Acpi::InterruptController* interrupt_controller = (Acpi::InterruptController*)(list + offset);

        switch (interrupt_controller->type) {
            case Acpi::InterruptControllerType::LAPIC: {
                Acpi::LocalApic* cpu = (Acpi::LocalApic*)interrupt_controller;

                if (cpu->flags & 1 && ((b >> 24) & 0xFF) != cpu->id)
                    Cpu::Smp::boot_cpu(cpu->id);
                break;
            }
        }

        offset += interrupt_controller->length;
    }
}