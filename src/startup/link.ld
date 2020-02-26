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

/*aarch64-elf always adds this section. Include in linker script so */
/*__task_rw_end aligns properly.                                    */
    .got  : {
        *(.got .got.*)
    }

    . = ALIGN(0x1000);
    .startup_list_header :
    {
        __startup_list_header = .;
        KEEP(*(.startup_list_header)) ;
    }
   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_sz = (__bss_end - __bss_beg)>>3;
 
