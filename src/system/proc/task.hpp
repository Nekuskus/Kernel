#pragma once
#include <stddef.h>
#include <stdint.h>

#include <system/cpu/cpu.hpp>
#include <system/mm/vmm.hpp>

namespace Tasking {
    enum class SchedulerStatus : uint16_t {
        BLOCKED,
        WORKING,
    };

    enum class ThreadState : uint16_t {
        SUSPENDED,
        ACTIVE,
        BLOCKED,
        WAITING,
    };

    struct Thread {
        ThreadState state;
        size_t tid;
        Cpu::Registers regs;
        uint64_t fs_base, gs_base;
        uint64_t kernel_rsp;
    };

    struct Process {
        size_t pid;
        const char *path;
        const char *cwd;
        Vmm::PageTable *cr3;
        LinkedList<Thread *> threads;
    };

    void init();
    void set_mask(bool mask);
    void switch_task(Cpu::Registers *registers, Thread *thread);
}  // namespace Tasking