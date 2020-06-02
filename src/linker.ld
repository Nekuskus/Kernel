ENTRY(_start)

SECTIONS {
    . = 0xFFFFFFFF80100000;
    _kernel_start = .;

    .stivalehdr : {
        *(.stivalehdr)
    }

    . = ALIGN(0x1000);

    .text : {
        *(.text*)
    }

    . = ALIGN(0x1000);

    .ctors : {
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    }

    .dtors : {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    }

    . = ALIGN(0x1000);

    .rodata : {
        *(.rodata*)
    }

    . = ALIGN(0x1000);

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON*)
    }

   _kernel_end = .;
}