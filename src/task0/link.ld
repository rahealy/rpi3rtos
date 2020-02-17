ENTRY(start)

SECTIONS
{
    . = 0x00000000;

    .task_list_item :
    {
        KEEP(*(.task_list_item)) ;
    }

    .task_header :
    {
        KEEP(*(.task_header)) ;
    }
    
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
        __task_end = .;
        __bss_beg = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_sz = (__bss_end - __bss_beg)>>3;
 
