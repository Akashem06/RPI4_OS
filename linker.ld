SECTIONS
{
    . = 0x80000;
    
    .text.boot : { 
        *(.text.boot) 
    }
    
    .text : { 
        __text_start = .;
        *(.text) 
    }
    
    . = ALIGN(8);
    .rodata : ALIGN(8) {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    }
    
    . = ALIGN(4096);
    .data : {
        __data_start = .;
        *(.data)
        . = ALIGN(4096);
        __data_end = .;
    }
    
    . = ALIGN(8);
    bss_begin = .;
    .bss : { 
        *(.bss*) 
    } 
    bss_end = .;
}