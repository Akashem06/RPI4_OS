#include "hardware.h"
#include "mem_utils.h"
#include "sysregs.h"

.section ".text.boot"
.globl _start
.globl __code_start

__code_start:
_start:
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, master
    b       proc_hang

master:
    ldr     x0, =SCTLR_VALUE_MMU_DISABLED
    msr     sctlr_el1, x0        

    ldr     x0, =HCR_VALUE
    msr     hcr_el2, x0

    ldr     x0, =SCR_VALUE
    msr     scr_el3, x0

    ldr     x0, =SPSR_VALUE
    msr     spsr_el3, x0

    adr     x0, el1_entry        
    msr     elr_el3, x0

    eret

el1_entry:
    // Remove rodata copy and just do BSS initialization
    adrp    x0, bss_begin
    add     x0, x0, :lo12:bss_begin

    adrp    x1, bss_end
    add     x1, x1, :lo12:bss_end
    sub     x1, x1, x0
    bl      memzero
    
    mov     sp, #LOW_MEMORY
    bl      kernel_main
    b       proc_hang

proc_hang:
    wfe
    b       proc_hang

.section ".text"
.globl __text_start
__text_start: