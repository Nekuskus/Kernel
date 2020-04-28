#pragma once
#include <stddef.h>
#include <stdint.h>

#include <lib/linked_list.hpp>
#include <system/mm/vmm.hpp>

#include "cpu.hpp"

namespace Cpu::Multitasking {
    struct Thread {
        size_t id;
        Cpu::Registers regs;
        uint64_t fs_base, gs_base;
        uint64_t user_rsp;
        uint64_t kernel_rsp;
    };

    struct Process {
        size_t id;
        char* path;
        char* cwd;
        Vmm::PageTable* cr3;
        LinkedList<Thread> threads;
    };

    void init();
    Thread* find_next_thread();
    void switch_task();
}  // namespace Cpu::Multitasking