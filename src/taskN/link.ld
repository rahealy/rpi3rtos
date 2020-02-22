SECTIONS
{
    . = 0x00000000;

    .task_list_item :
    {
        KEEP(*(.task_list_item)) ;
        __task_end = .;
    }

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
