#pragma once
#include <stddef.h>
#include <stdint.h>

#include <system/mm/vmm.hpp>

#include "cpu.hpp"

namespace Cpu::Multitasking {
    enum ThreadState : uint16_t {
        ACTIVE = 1,
        SUSPENDED,
        BLOCKED,
        WAITING,
    };

    struct Thread {
        ThreadState state;
        size_t tid;
        Cpu::Registers regs;
        uint64_t fs_base, gs_base;
        uint64_t user_rsp;
        uint64_t kernel_rsp;
    };

    struct Process {
        size_t pid;
        char* path;
        char* cwd;
        Vmm::PageTable* cr3;
        LinkedList<Thread*> threads;
    };

    void init();
    void switch_task();
}  // namespace Cpu::Multitasking