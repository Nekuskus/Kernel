#include "task.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/cpu/apic.hpp>
#include <system/debugging.hpp>
#include <system/drivers/time.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <system/terminal.hpp>

static constexpr inline Cpu::Registers default_kernel_regs{
    .cs = 0x08,
    .rflags = 0x202,
    .ss = 0x10,
};

/* for later
static constexpr inline Cpu::Registers default_user_regs{
    .cs = 0x23,
    .rflags = 0x202,
    .ss = 0x1b,
};*/

static auto processes = LinkedList<Tasking::Process*>();

inline Tasking::Thread* find_next_thread() {
    auto current_cpu = Cpu::get_current_cpu();
    auto kernel_idle_thread = *(*processes[0])->threads[0];
    auto current_process_threads_len = (*processes[current_cpu->process])->threads.length();

    for (size_t i = 0; i < current_process_threads_len; i++) {
        if (current_cpu->thread >= current_process_threads_len) {
            if (++current_cpu->process >= processes.length())
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

        if (next_thread->state != Tasking::ThreadState::WAITING) {
            current_cpu->thread++;

            continue;
        }

        next_thread->state = Tasking::ThreadState::ACTIVE;
        current_cpu->thread = next_thread->tid;

        return next_thread;
    }

    kernel_idle_thread->state = Tasking::ThreadState::ACTIVE;
    current_cpu->process = 0;
    current_cpu->thread = 0;

    return kernel_idle_thread;
}

void Tasking::switch_task(Cpu::Registers* registers, Thread* thread) {
    auto current_cpu = Cpu::get_current_cpu();

    if (current_cpu->process != 0) {
        auto old_thread = *(*processes[current_cpu->process])->threads[current_cpu->thread];
        old_thread->regs = *registers;
        old_thread->fs_base = Cpu::read_msr(fs_base);
        old_thread->gs_base = Cpu::read_msr(gs_base);
    }

    *registers = thread->regs;

    Cpu::write_msr(fs_base, thread->fs_base);
    Cpu::write_msr(gs_base, thread->gs_base);
}

void schedule(Cpu::Registers* registers) {
    auto thread = find_next_thread();

    Tasking::switch_task(registers, thread);
}

void kernel_idle_task() {
    asm volatile(
        "sti;"
        "1: "
        "hlt;"
        "jmp 1b;");
}

void Tasking::init() {
    if (processes.length() < 1) {
        Debug::print("[Multitasking] Creating PID 0 (Kernel Idle Task).\n\r");

        Process* kernel_idle = new Process;

        processes.push_back(kernel_idle);

        kernel_idle->cr3 = Vmm::get_ctx_kernel();
        kernel_idle->pid = 0;
        kernel_idle->cwd = kernel_idle->path = nullptr;

        auto kernel_idle_thread = new Thread;
        kernel_idle_thread->tid = 0;
        kernel_idle_thread->regs = default_kernel_regs;
        kernel_idle_thread->regs.rip = (uint64_t)kernel_idle_task;
        kernel_idle_thread->kernel_rsp = (uint64_t)calloc(0x10000, 1) + 0x10000;
        kernel_idle_thread->user_rsp = kernel_idle_thread->regs.rsp = (uint64_t)calloc(0x10000, 1) + 0x10000;

        kernel_idle->threads.push_back(kernel_idle_thread);
    }

    Cpu::Apic::LocalApic::write(Cpu::Apic::LocalApic::TimerRegisters::TIMER_LVT, 232 | (1 << 17));
    Cpu::Apic::LocalApic::write(Cpu::Apic::LocalApic::TimerRegisters::TIMER_DIV, 0x3);
    Cpu::Apic::LocalApic::write(Cpu::Apic::LocalApic::TimerRegisters::TIMER_INITCNT, 0xFFFFFFFF);

    Time::ksleep(20);

    uint32_t ticks = 0xFFFFFFFF - Cpu::Apic::LocalApic::read(Cpu::Apic::LocalApic::TimerRegisters::TIMER_CURRCNT);

    Cpu::Apic::LocalApic::write(Cpu::Apic::LocalApic::TimerRegisters::TIMER_INITCNT, ticks / 10);

    Idt::register_interrupt_handler(232, schedule, true, true);

    char debug[255] = "";

    sprintf(debug, "[Multitasking] Initialized scheduler on CPU #%d.\n\r", Cpu::get_current_cpu()->id);
    Debug::print(debug);
}