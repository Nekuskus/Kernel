section .trampoline

bits 16

KERNEL_VMA equ 0xFFFFFFFF80000000

extern kernel_pml4

extern smp_kernel_main

global smp_entry
smp_entry:
    cli
    cld
    jmp 0x000:.reset_cs
.reset_cs:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    lgdt [gdt32_ptr]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:.prot_mode
.prot_mode:
bits 32
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    lgdt [gdt64_ptr]
    mov esp, [trampoline_stack - KERNEL_VMA]

    mov eax, cr4
    or eax, 0x000000A0
    mov cr4, eax

    mov eax, [kernel_pml4 - KERNEL_VMA]
    mov cr3, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x00000901
    wrmsr

    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    mov ecx, 0x277
    mov eax, 0x05010406
    xor edx, edx
    wrmsr

    jmp 0x08:.long_mode
.long_mode:
bits 64
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    add rsp, KERNEL_VMA

    cld

    call smp_kernel_main
    hlt

gdt32_start:
    dd 0x0
    dd 0x0
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt32_end:

gdt32_ptr:
    dw gdt32_end - gdt32_start - 1
    dd gdt32_start

gdt64_start:
    dq 0
    dq 0x00209A0000000000
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start

gdt64_ptr_high:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start + 0xFFFF800000000000

section .data
global trampoline_stack
trampoline_stack:
    dq 0