section .trampoline

bits 16

KERNEL_VMA equ 0xFFFFFFFF80000000

extern smp_kernel_main

global smp_entry
smp_entry:
    cli
    cld

    xor ax, ax
    mov ds, ax

    jmp 0x0:fix_cs
    fix_cs:
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    lgdt [gdt_ptr]

    mov edx, dword [0x510]
    mov cr3, edx

    mov eax, cr4
    or eax, 1 << 5
    or eax, 1 << 7
    mov cr4, eax

    mov ecx, 0xc0000080
    rdmsr

    or eax, 0x00000100
    wrmsr

    mov eax, cr0
    or eax, 0x80000001
    and eax, ~(0x60000000)
    mov cr0, eax

    jmp 0x08:.mode64
    .mode64:
    bits 64
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, [trampoline_stack]

    lgdt [0x520]
    lidt [0x530]

    mov ax, 0x38
    ltr ax

    cld

    call smp_kernel_main

align 16

gdt_ptr:
    dw .gdt_end - .gdt_start - 1
    dq .gdt_start

align 16
.gdt_start:

.null_descriptor:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 00000000b
    db 00000000b
    db 0x00

.kernel_code_64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10011010b
    db 00100000b
    db 0x00

.kernel_data:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10010010b
    db 00000000b
    db 0x00

.user_data_64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 11110010b
    db 00000000b
    db 0x00

.gdt_end:

bits 64

section .text

global prepare_trampoline
prepare_trampoline:
    mov qword [0x510], rdi
    sgdt [0x520]
    sidt [0x530]
    ret

section .data

global trampoline_stack
trampoline_stack:
    dq 0