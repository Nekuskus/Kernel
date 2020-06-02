bits 64
section .text

global trampoline
global trampoline_end
global trampoline_size

%define _trampoline_size trampoline_end - trampoline_end

trampoline:
    incbin "src/system/cpu/smp/trampoline.bin"
trampoline_end:

trampoline_size: dq _trampoline_size

global prepare_trampoline
prepare_trampoline:
    mov qword [0x510], rdi
    sgdt [0x520]
    sidt [0x530]
    mov qword [0x540], rsi
    mov qword [0x550], rdx
    ret