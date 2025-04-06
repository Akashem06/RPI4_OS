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

    /* Heap starts after BSS and grows upward */
    . = ALIGN(4096);
    __heap_start = .;
    . = . + 2M; /* 2MB heap size */
    __heap_end = .;

    /* 1 Guard page to protect against heap overflow into stack */
    . = ALIGN(4096);
    __guard_page_start = .;
    . = . + 4096;
    __guard_page_end = .;

    /* Stack grows downward from top of memory */
    . = ALIGN(16);
    __stack_bottom = .;
    . = . + 1M;     /* 1MB stack size */
    __stack_top = .;

    ASSERT(__stack_top <= 0x80000 + 16M, "Memory layout exceeds 16MB!")

    __low_memory = 0x1000000;   /* 16MB - after kernel space */
    __high_memory = 0x10000000; /* 256MB for user space */
}