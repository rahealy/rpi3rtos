SECTIONS
{
    . = 0x80000;
    
    .text : {
        KEEP(*(.text.start)) *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    . = ALIGN(0x1000);

    .data : {
        __ro_end = .;
        *(.data .data.*)
    }

    .bss ALIGN(0x10):
    {
        __bss_beg = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    .task_list :
    {
        KEEP(*(.task_list)) ;
    }

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_sz = (__bss_end - __bss_beg)>>3;
