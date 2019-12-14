bits 64

%macro pushfregs 0
    push rax
    push rcx
    push rdx
    push rbx
    push rsp
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov ax, ds
    movzx rax, ax
    push rax
 
    mov ax, 0x0
    mov ds, ax
    mov es, ax
   
    cld
    mov rdi, rsp
%endmacro
 
%macro popfregs 0
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
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16
	iretq
%endmacro

extern idt_page_fault

global page_fault_handler
page_fault_handler:
	pushfregs
	call idt_page_fault
    popfregs