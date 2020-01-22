#include "madt.hpp"
#include <stddef.h>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/terminal.hpp>
#include <lib/lib.hpp>
#include "apic.hpp"

static auto lapics = LinkedList<Madt::LocalApic*>();
static auto ioapics = LinkedList<Madt::IoApic*>();
static auto isos = LinkedList<Madt::InterruptSourceOverride*>();

static bool legacy_pic = false;

LinkedList<Madt::LocalApic*> Madt::get_lapics() {
    auto result = LinkedList<Madt::LocalApic*>();

    for (auto& lapic : lapics)
        result.push_back(lapic);

    return result;
}

LinkedList<Madt::IoApic*> Madt::get_ioapics() {
    auto result = LinkedList<Madt::IoApic*>();

    for (auto& ioapic : ioapics)
        result.push_back(ioapic);

    return result;
}

LinkedList<Madt::InterruptSourceOverride*> Madt::get_isos() {
    auto result = LinkedList<Madt::InterruptSourceOverride*>();

    for (auto& iso : isos)
        result.push_back(iso);

    return result;
}

bool Madt::has_legacy_pic() {
    return legacy_pic;
}

void Madt::init() {
    MadtHeader* madt = (MadtHeader*)Acpi::get_table("APIC");

    legacy_pic = (madt->flags & 1ULL) == 1;

    size_t table_size = madt->header.length - sizeof(MadtHeader);
    uint64_t list = (uint64_t)madt + sizeof(MadtHeader), offset = 0;

    while (offset < table_size) {
        InterruptController* interrupt_controller = (InterruptController*)(list + offset);

        switch (interrupt_controller->type) {
            case InterruptControllerType::LAPIC: {
                LocalApic* lapic = (LocalApic*)interrupt_controller;

                lapics.push_back(lapic);

                break;
            }

            case InterruptControllerType::IOAPIC: {
                IoApic* ioapic = (IoApic*)interrupt_controller;

                Vmm::map_pages(Vmm::get_current_context(), (void*)((uint64_t)ioapic->ioapic_base + virtual_kernel_base), (void*)(uint64_t)ioapic->ioapic_base, 1, Vmm::VirtualMemoryFlags::VMM_PRESENT | Vmm::VirtualMemoryFlags::VMM_WRITE);
                ioapics.push_back(ioapic);

                break;
            }

            case InterruptControllerType::INTERRUPT_SOURCE_OVERRIDE: {
                InterruptSourceOverride* iso = (InterruptSourceOverride*)interrupt_controller;

                isos.push_back(iso);

                break;
            }
        }

        offset += interrupt_controller->length;
    }
}