#include "smp.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/acpi/madt.hpp>
#include <system/cpu/apic.hpp>
#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/mm/mm.hpp>
#include <system/mm/vmm.hpp>
#include <system/proc/task.hpp>

extern "C" uint64_t trampoline_size;
extern "C" void *stack_end;

extern "C" void init_bsp_local(void *);
extern "C" void prepare_trampoline(void *, void (*)(), void *, void *);

static bool wait_for_boot() {
    for (int i = 0; i < 1000; i++) {
        Time::ksleep(1);

        if (*(bool *)0x510)
            return true;
    }

    return false;
}

static void ap_entry() {
    Tasking::init();
    *(bool *)0x510 = true;

    asm volatile("sti");

    for (;;)
        asm volatile("hlt");
}

void Cpu::Smp::init() {
    auto current_cpu = Cpu::get_current_cpu();

    Tss *bsp_tss = current_cpu->tss = new Tss;
    bsp_tss->rsp[0] = (uint64_t)&stack_end;

    init_bsp_local(bsp_tss);

    auto kernel_pml4 = Vmm::get_ctx_kernel();

    Vmm::map_pages(kernel_pml4, (void *)0x1000, (void *)0x1000, (trampoline_size + 0x1000 - 1) / 0x1000, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);
    Vmm::map_pages(kernel_pml4, (void *)0x510, (void *)0x510, 1, (int)Vmm::VirtualMemoryFlags::PRESENT | (int)Vmm::VirtualMemoryFlags::WRITE);

    for (auto lapic : Madt::lapics) {
        if (((lapic->flags & 1) || (lapic->flags & 2)) && lapic->id != current_cpu->id) {
            auto trampoline_stack = calloc(0x10000, 1);
            char debug[255] = "";

            if (!trampoline_stack) {
                sprintf(debug, "[SMP] Failed to allocate stack for CPU with lapic ID %d\n\r", lapic->id);
                Debug::print(debug);

                return;
            }

            sprintf(debug, "[SMP] Allocated stack for CPU with lapic ID %d\n\r", lapic->id);
            Debug::print(debug);

            uint64_t trampoline_stack_end = (uint64_t)trampoline_stack + 0x10000;

            auto cpu = new CpuState;
            cpu->id = lapic->id;

            push(cpu);

            Tss *tss = cpu->tss = new Cpu::Tss;
            tss->rsp[0] = trampoline_stack_end;

            *(bool *)0x510 = false;
            prepare_trampoline(kernel_pml4, ap_entry, (void *)trampoline_stack_end, tss);

            Apic::LocalApic::send_ipi(lapic->id, 0x500);
            Apic::LocalApic::send_ipi(lapic->id, 0x600 | 1);

            if (!wait_for_boot())
                Apic::LocalApic::send_ipi(lapic->id, 0x600 | 1);

            if (wait_for_boot()) {
                cpu->booted = true;

                sprintf(debug, "[SMP] Booted CPU with lapic ID #%d\n\r", lapic->id);
                Debug::print(debug);
            } else {
                sprintf(debug, "[SMP] Failed to boot CPU with lapic ID #%d\n\r", lapic->id);
                Debug::print(debug);
            }
        }
    }

    Debug::print("[SMP] Finished setting up.\n\r");
}