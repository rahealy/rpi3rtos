SECTIONS
{
    . = 0x80000;
    .text : { 
        KEEP(*(.text.start)) *(.text .text.*)
    }

    .rodata :
    {
        *(.rodata .rodata.*)
    }

    .data : { 
        *(.data .data.*)
    }
    
    .bss ALIGN(16):
    {
        __bss_beg = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_sz = (__bss_end - __bss_beg)>>3;
