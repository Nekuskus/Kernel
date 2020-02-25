bits 64

section .init
global _init:function hidden
_init:

section .fini
global _fini:function hidden
_fini:

section .ctors
global __CTOR_LIST__:data hidden
__CTOR_LIST__:

section .dtors
global __DTOR_LIST__:data hidden
__DTOR_LIST__:
