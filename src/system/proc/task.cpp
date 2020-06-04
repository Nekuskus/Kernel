#include "task.hpp"

#include <lib/lib.hpp>
#include <lib/linked_list.hpp>
#include <system/cpu/apic.hpp>
#include <system/debugging.hpp>
#include <system/idt.hpp>
#include <system/mm/mm.hpp>
#include <lib/spinlock.hpp>

static constexpr inline Cpu::Registers default_kernel_regs{
    .cs = 0x08,
    .rflags = 0x202,
    .ss = 0x10,
};

// for later
static constexpr inline Cpu::Registers default_user_regs [[maybe_unused]]{
    .cs = 0x23,
    .rflags = 0x202,
    .ss = 0x1b,
};

static auto scheduler_mask = true;
static auto processes = LinkedList<Tasking::Process *>();
//static auto threads = LinkedList<Tasking::Thread *>();
static uint32_t lock = 0;


static Tasking::Thread *find_next_thread() {
    auto current_cpu = Cpu::get_current_cpu();
    auto kernel_idle_thread = *(*processes[0])->threads[0];

    kernel_idle_thread->state = Tasking::ThreadState::ACTIVE;
    current_cpu->process = 0;
    current_cpu->thread = 0;

    return kernel_idle_thread;
}

static void schedule(Cpu::Registers *registers) {
    if (scheduler_mask)
        return;

    acquire_lock(&lock);
    Tasking::switch_task(registers, find_next_thread());
    release_lock(&lock);
}

static void kernel_idle_task() {
    asm volatile(
        "sti;"
        "1: "
        "hlt;"
        "jmp 1b;");
}

void Tasking::switch_task(Cpu::Registers *registers, Thread *thread) {
    auto current_cpu = Cpu::get_current_cpu();

    *registers = thread->regs;

    Cpu::write_msr(fs_base, thread->fs_base);
    Cpu::write_msr(gs_base, thread->gs_base);

    Vmm::set_context((*processes[current_cpu->process])->cr3);
}

void Tasking::set_mask(bool mask) {
    scheduler_mask = mask;
}

void Tasking::init() {
    if (processes.length() < 1) {
        Idt::register_interrupt_handler(128, schedule, true, true);

        Debug::print("[Tasking] Creating PID 0 (Kernel Idle Task).\n\r");

        Process *kernel_idle = new Process;

        processes.push_back(kernel_idle);

        kernel_idle->cr3 = Vmm::get_ctx_kernel();
        kernel_idle->pid = 0;
        kernel_idle->cwd = kernel_idle->path = nullptr;

        auto kernel_idle_thread = new Thread;
        kernel_idle_thread->tid = 0;
        kernel_idle_thread->regs = default_kernel_regs;
        kernel_idle_thread->regs.rip = (uint64_t)kernel_idle_task;
        kernel_idle_thread->kernel_rsp = (uint64_t)calloc(0x10000, 1) + 0x10000;
        kernel_idle_thread->regs.rsp = (uint64_t)calloc(0x10000, 1) + 0x10000;

        kernel_idle->threads.push_back(kernel_idle_thread);
    }

    Cpu::Apic::LocalApic::enable_timer();

    char debug[255] = "";

    sprintf(debug, "[Tasking] Initialized scheduler on CPU #%d.\n\r", Cpu::get_current_cpu()->id);
    Debug::print(debug);
}