SECTIONS {
    . = 0x80000;
    
    .text.boot : {
        *(.text.boot)
    }
    
    .text : {
        __text_start = .;
        *(.text)
    }
    
    . = ALIGN(8);
    .rodata : {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    }
    
    . = ALIGN(16);
    .data : {
        __data_start = .;
        *(.data)
        . = ALIGN(16);  /* Changed from 4096 */
        __data_end = .;
    }
    
    /* No alignment between data and bss to keep them close */
    .bss : {
        __bss_begin = .;
        *(.bss*)
        . = ALIGN(8);
        __bss_end = .;
    }
}