ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFF80000000 + 0x200000;
    _kernel_start = .;

    .stivalehdr ALIGN(4K) :
    {
        KEEP(*(.stivalehdr))
    }

    .text ALIGN(4K) :
    {
        KEEP(*(.text*))
    }

    .rodata ALIGN(4K) :
    {
        KEEP(*(.rodata*))
    }

    .data ALIGN(4K) :
    {
        KEEP(*(.data*))
    }

    .bss ALIGN(4K) :
    {
        KEEP(*(COMMON))
        KEEP(*(.bss*))
    }

   _kernel_end = .;
}