global trampoline_size
global prepare_trampoline
global check_ap_flag
global init_bsp_local

extern load_tss

section .data

%define _trampoline_size trampoline_end - trampoline

trampoline:
    incbin "src/system/cpu/smp/trampoline.bin"
trampoline_end:
trampoline_size: dq _trampoline_size

section .text

prepare_trampoline:
    mov byte [0x510], 0
    mov qword [0x520], rdi
    sgdt [0x530]
    sidt [0x540]
    mov qword [0x550], rsi
    mov qword [0x560], rdx

    mov rsi, trampoline
    mov rdi, 0x1000
    mov rcx, _trampoline_size
    rep movsb

    mov rdi, rcx
    call load_tss

    ret

check_ap_flag:
    xor rax, rax
    mov al, byte [0x510]
    ret

init_bsp_local:
    call load_tss

    mov ax, 0x38
    ltr ax

    ret