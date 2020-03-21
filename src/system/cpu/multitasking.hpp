#pragma once
#include <lib/linked_list.hpp>
#include <system/mm/vmm.hpp>

namespace Cpu::Multitasking {
    struct Thread {
        size_t id;
        uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp;
        uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
        uint64_t rip;
        uint64_t ss, ds, cs;
        uint64_t fs_base, gs_base;
        uint64_t rflags;
    };

    struct Process {
        size_t id;
        char* title;
        char* path;
        char* cwd;
        Vmm::PageTable* cr3;
        LinkedList<Thread> threads;
    };

    void init();
}  // namespace Cpu::Multitasking