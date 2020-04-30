#include "multitasking.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/terminal.hpp>

#include "apic.hpp"

static auto processes = LinkedList<Cpu::Multitasking::Process*>();

inline Cpu::Multitasking::Thread* find_next_thread() {
    auto current_cpu = Cpu::get_current_cpu();
    auto kernel_idle_thread = *(*processes[0])->threads[0];
    int current_process_threads_len = (*processes[current_cpu->process])->threads.length();

    for (int i = 0; i < current_process_threads_len; i++) {
        if (current_cpu->thread >= current_process_threads_len) {
            if (++current_cpu->process >= (int)processes.length())
                current_cpu->process = 1;

            current_cpu->thread = 1;
        }

        auto process = processes[current_cpu->process];

        if (!process) {
            current_cpu->process++;

            continue;
        }

        auto thread = (*process)->threads[current_cpu->thread];

        if (!thread) {
            current_cpu->thread++;

            continue;
        }

        auto next_thread = *thread;

        if (next_thread->state != Cpu::Multitasking::ThreadState::WAITING) {
            current_cpu->thread++;

            continue;
        }

        next_thread->state = Cpu::Multitasking::ThreadState::ACTIVE;
        current_cpu->thread = next_thread->tid;

        return next_thread;
    }

    kernel_idle_thread->state = Cpu::Multitasking::ThreadState::ACTIVE;
    current_cpu->process = 0;
    current_cpu->thread = 0;

    return kernel_idle_thread;
}

void Cpu::Multitasking::switch_task() {
    [[maybe_unused]] Thread* thread = find_next_thread();
}

void schedule([[maybe_unused]] const Cpu::Registers* registers) {
    Cpu::Multitasking::switch_task();
}

void kernel_idle_task() {
    asm volatile(
        "sti;"
        "1: "
        "hlt;"
        "jmp 1b;");
}

void Cpu::Multitasking::init() {
    if (processes.length() < 1) {
        Debug::print("[Multitasking] Creating PID 0 (Kernel Idle Task).\n\r");

        Process* kernel_idle = new Process;

        processes.push_back(kernel_idle);

        kernel_idle->cr3 = Vmm::get_ctx_kernel();
        kernel_idle->pid = 0;
        kernel_idle->cwd = kernel_idle->path = nullptr;

        Thread* kernel_idle_thread = new Thread;
        kernel_idle_thread->tid = 0;
        kernel_idle_thread->regs.rip = (uint64_t)kernel_idle_task;
        kernel_idle_thread->kernel_rsp = (uint64_t)calloc(0x10000, 1) + 0x10000;
        kernel_idle_thread->user_rsp = (uint64_t)calloc(0x10000, 1) + 0x10000;

        kernel_idle->threads.push_back(kernel_idle_thread);
    }

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_LVT, 232 | (1 << 17));
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_DIV, 0x3);
    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, 0xFFFFFFFF);

    Time::ksleep(20);

    uint32_t ticks = 0xFFFFFFFF - Apic::LocalApic::read(Apic::LocalApic::TimerRegisters::TIMER_CURRCNT);

    Apic::LocalApic::write(Apic::LocalApic::TimerRegisters::TIMER_INITCNT, ticks / 10);

    Idt::register_interrupt_handler(232, schedule, true, true);

    char debug[255] = "";

    sprintf(debug, "[Multitasking] Initialized scheduler on CPU #%d.\n\r", Cpu::get_current_cpu()->id);
    Debug::print(debug);
}