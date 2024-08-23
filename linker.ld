SECTIONS
{
    .text.boot : { *(.text.boot) }
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    bss_begin = .;
    .bss : { *(.bss*) } 
    bss_end = .;
}

/* The RPI loads our .img file into memory and executes from the begining of the file */
/* This is why we want the .text.boot section first, the OS startup code is located here. */
/* .text, .rodata and .data sections are kernal compiled instructions that aren't important */
/* .bss is unitialized values that are all set to 0 */
