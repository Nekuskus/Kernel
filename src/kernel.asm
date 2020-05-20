KERNEL_VMA equ 0xFFFFFFFF80000000

bits 32

section .multiboot

mb_flags equ (1 << 0) | (1 << 1) | (1 << 2)

align 64
mb_header:
    dd 0x1BADB002
    dd mb_flags
    dd -0x1BADB002 - mb_flags
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 1280
    dd 720
    dd 32

section .data

global kernel_pml4
align 0x1000
kernel_pml4:
    dq init_pdp1 - KERNEL_VMA + 3
    times 255 dq 0
    dq phys_pdp - KERNEL_VMA + 3
    times 254 dq 0
    dq init_pdp2 - KERNEL_VMA + 3

%macro gen_pd_2mb 3
    %assign i %1
    %rep %2
        dq (i | 0x83)
        %assign i i + 0x200000
    %endrep
    %rep %3
        dq 0
    %endrep
%endmacro

align 0x1000
init_pdp1:
    dq init_pd - KERNEL_VMA + 3
    times 511 dq 0

align 0x1000
init_pdp2:
    times 510 dq 0
    dq init_pd - KERNEL_VMA + 3
    dq 0

align 0x1000
phys_pdp:
    dq phys_pd - KERNEL_VMA + 3
    times 511 dq 0

align 0x1000
init_pd:
    gen_pd_2mb 0, 64, 448

align 0x1000
phys_pd:
    gen_pd_2mb 0, 512, 0

align 0x10
init_gdt:
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

init_gdt_end:

global init_gdt_ptr
init_gdt_ptr:
    dw init_gdt_end - init_gdt - 1
    dq init_gdt - KERNEL_VMA

section .text

global loader
loader:
    cli
    lgdt [init_gdt_ptr - KERNEL_VMA]
    mov esp, stack_end - KERNEL_VMA
    push 0
    push eax
    push 0
    push ebx
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb stop
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz stop
    mov eax, cr4
    or eax, 0x000000A0
    mov cr4, eax
    mov eax, kernel_pml4 - KERNEL_VMA
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
    jmp 0x08:higher_half_entry - KERNEL_VMA

bits 64

global init_gdt_ptr_high
init_gdt_ptr_high:
    dw init_gdt_end - init_gdt - 1
    dq init_gdt

global load_tss

load_tss:
    push rbx
    mov eax, edi
    mov rbx, init_gdt.tss_low
    mov word [rbx], ax
    mov eax, edi
    and eax, 0xff0000
    shr eax, 16
    mov rbx, init_gdt.tss_mid
    mov byte [rbx], al
    mov eax, edi
    and eax, 0xff000000
    shr eax, 24
    mov rbx, init_gdt.tss_high
    mov byte [rbx], al
    mov rax, rdi
    shr rax, 32
    mov rbx, init_gdt.tss_upper32
    mov dword [rbx], eax
    mov rbx, init_gdt.tss_flags1
    mov byte [rbx], 10001001b
    mov rbx, init_gdt.tss_flags2
    mov byte [rbx], 0
    pop rbx
    ret

higher_half_entry:
    mov rax, entry
    jmp rax

extern kmain
entry:
    lgdt [init_gdt_ptr_high]
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    add rsp, KERNEL_VMA

    extern _init
    call _init

    pop rdi
    pop rsi

    cld

    call kmain

    extern _fini
    call _fini

    cli
    hlt

global stop
stop:
    cli
    hlt

section .bss
align 0x1000
stack:
    resb 0x10000

global stack_end
stack_end: