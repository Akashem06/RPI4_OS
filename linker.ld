SECTIONS
{
    . = 0x0;
    __code_start = .;
    .text.boot : { 
        *(.text.boot) 
    }
    __text_start = .;
    .text : { 
        *(.text) 
    }
    .rodata : { 
        *(.rodata) 
    }
    .data : ALIGN(4096) {
        __data_start = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4096);
        __data_end = .;
    }
    bss_begin = .;
    .bss : { 
        *(.bss*) 
    } 
    bss_end = .;
}