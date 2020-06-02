bits 64

section .data

align 0x10
gdt:
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

.user_code_64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 11111010b
    db 00100000b
    db 0x00

.unreal_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 10001111b
    db 0x00

.unreal_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 10001111b
    db 0x00

.tss:
    dw 104
.tss_low:
    dw 0
.tss_mid:
    db 0
.tss_flags1:
    db 10001001b
.tss_flags2:
    db 00000000b
.tss_high:
    db 0
.tss_upper32:
    dd 0
.tss_reserved:
    dd 0

gdt_end:

global gdt_ptr
gdt_ptr:
    dw gdt_end - gdt - 1
    dq gdt

section .text

global load_tss

load_tss:
    push rbx
    mov eax, edi
    mov rbx, gdt.tss_low
    mov word [rbx], ax
    mov eax, edi
    and eax, 0xff0000
    shr eax, 16
    mov rbx, gdt.tss_mid
    mov byte [rbx], al
    mov eax, edi
    and eax, 0xff000000
    shr eax, 24
    mov rbx, gdt.tss_high
    mov byte [rbx], al
    mov rax, rdi
    shr rax, 32
    mov rbx, gdt.tss_upper32
    mov dword [rbx], eax
    mov rbx, gdt.tss_flags1
    mov byte [rbx], 10001001b
    mov rbx, gdt.tss_flags2
    mov byte [rbx], 0
    pop rbx
    ret

global init_gdt
init_gdt:
    lgdt [gdt_ptr]
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

section .bss
align 0x1000
stack:
    resb 0x1000

global stack_end
stack_end: