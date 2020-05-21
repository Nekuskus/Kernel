extern isr_handler

%macro ISR_NOERROR 1
isr%1:
    cli
    push 0
    push %1
    jmp isr_stub
%endmacro

%macro ISR_ERROR 1
isr%1:
    cli
    push %1
    jmp isr_stub
%endmacro

%assign i 0
%rep 8
ISR_NOERROR i
%assign i i + 1
%endrep
ISR_ERROR 8
ISR_NOERROR 9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
%assign i 15
%rep 240
ISR_NOERROR i
%assign i i + 1
%endrep

isr255:
    iretq

%macro GET_ISR_ADDR 1
dq isr%1
%endmacro

global isrs
isrs:
    %assign i 0
    %rep 256
    GET_ISR_ADDR i
    %assign i i + 1
    %endrep

isr_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    xor rax, rax
    mov ax, ds
    push rax
    mov ax, 0x0
    mov ds, ax
    mov es, ax

    cld
    mov rdi, rsp

    call isr_handler

    pop rax
    mov ds, ax
    mov es, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq