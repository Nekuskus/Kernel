bits 64

extern __fw_ctors
extern __fw_dtors

section .init
    call __fw_ctors
    ret

section .fini
    call __fw_dtors
    ret

section .ctors
global __CTOR_END__:data hidden
__CTOR_END__:

section .dtors
global __DTOR_END__:data hidden
__DTOR_END__:
