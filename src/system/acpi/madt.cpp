#include "madt.hpp"

#include <stddef.h>

#include <lib/lib.hpp>
#include <system/debugging.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/panic.hpp>

auto Madt::lapics = LinkedList<Madt::LocalApic *>();
auto Madt::ioapics = LinkedList<Madt::IoApic *>();
auto Madt::isos = LinkedList<Madt::InterruptSourceOverride *>();
bool Madt::legacy_pic = false;
Madt::MadtHeader *Madt::madt = nullptr;

void Madt::init() {
    madt = (MadtHeader *)Acpi::get_table("APIC");

    if (!madt)
        panic("Your hardware is unsupported, usually computers from this era have this MADT table in ACPI, but yours doesn't!");

    legacy_pic = madt->flags & 1ULL;

    size_t table_size = madt->header.length - sizeof(MadtHeader);
    uint64_t list = (uint64_t)madt + sizeof(MadtHeader), offset = 0;

    while (offset < table_size) {
        InterruptController *interrupt_controller = (InterruptController *)(list + offset);

        switch (interrupt_controller->type) {
            case InterruptControllerType::LAPIC: {
                LocalApic *lapic = (LocalApic *)interrupt_controller;

                lapics.push_back(lapic);

                break;
            }

            case InterruptControllerType::IOAPIC: {
                IoApic *ioapic = (IoApic *)interrupt_controller;

                Vmm::map_pages(Vmm::get_ctx_kernel(), (void *)((uint64_t)ioapic->ioapic_base + virtual_kernel_base), (void *)(uint64_t)ioapic->ioapic_base, 1, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);
                ioapics.push_back(ioapic);

                break;
            }

            case InterruptControllerType::INTERRUPT_SOURCE_OVERRIDE: {
                InterruptSourceOverride *iso = (InterruptSourceOverride *)interrupt_controller;

                isos.push_back(iso);

                break;
            }
        }

        offset += interrupt_controller->length;
    }

    Debug::print("[MADT] Finished setting up.\n\r");
}