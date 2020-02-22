ENTRY(start)

SECTIONS
{
    . = 0x00000000;

/*Task list item always first structure in r/o memory.*/ 
    .task_list_item : /*32 bytes*/
    {
        KEEP(*(.task_list_item)) ;
    }

    
/*R/O executable code*/ 
    .text : {
        KEEP(*(.text.start)) *(.text .text.*)
    }

    .rodata : {
        __task_ro_beg = .;
        *(.rodata .rodata.*)
        __task_ro_end = .;
    }

/*R/W memory 4kB aligned to fit in the next 4kB page table block.*/
    . = ALIGN(0x1000);
    
/*Task header always first structure in 4kB aligned r/w memory.*/ 
    .task_header :
    {
        __task_rw_beg = .;
        __task_header_beg = .;
        *(.task_header) ;
    }

/*Initialized r/w memory.*/
    .data : {
        *(.data .data.*)
    }

/*aarch64-elf always adds this section. Include in linker script so */
/*__task_rw_end aligns properly.                                    */
    .got  : {
        *(.got .got.*)
    }

    __task_rw_end = .;

/*Uninitialized r/w memory.*/
    .bss ALIGN(0x10):
    {
        __task_bss_beg = .;
        *(.bss .bss.*)
        *(COMMON)
        __task_bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__task_bss_sz = (__task_bss_end - __task_bss_beg)>>3;
 
