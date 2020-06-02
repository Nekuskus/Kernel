#pragma once
#include <stddef.h>
#include <stdint.h>

#include <lib/linked_list.hpp>

constexpr uint32_t fs_base = 0xC0000100;
constexpr uint32_t gs_base = 0xC0000101;

namespace Cpu {
    struct [[gnu::packed]] Tss {
        uint32_t unused1;
        uint64_t rsp[4];
        uint64_t unused2;
        uint64_t ist[7];
        uint64_t unused3;
        uint16_t unused4;
        uint16_t io_bitmap_offset;
        uint8_t io_bitmap[8193];
    };

    struct Registers {
        uint64_t ds;
        uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
        uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
        uint64_t int_num, error_code;
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    };

    struct CpuState {
        bool booted;
        Tss *tss;
        uint32_t id;
        size_t process;
        size_t thread;
    };

    bool check_msr(uint32_t flag);
    uint64_t read_msr(uint32_t msr);
    void write_msr(uint32_t msr, uint64_t value);
    void halt_forever();
    void push(CpuState *cpu_state);
    CpuState *get_current_cpu();
    LinkedList<CpuState *> &get_cpu_states();
}  // namespace Cpu