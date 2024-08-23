#include "mm.h"
#include "sysregs.h"

// Specifies that this is .text.boot, the first item in the linkerscript and image
// Ensures that this file runs first, specifically the start function
.section ".text.boot"

.globl _start
// Checking the processor ID
_start:
    mrs x0, mpidr_el1 // Moves mpidr_el1 into x0
    and x0, x0, #0xFF // Applies AND mask, x0 & 0xFF to keep lower byte

    // Compare and branch if Zero
    // If x0 is 0, branches to master function
    // If not, it continues
    cbz x0, master

    b proc_hang // Program hangs

master:
    // Look at sysregs.h for more details!
    ldr    x0, =SCTLR_VALUE_MMU_DISABLED
    msr    sctlr_el1, x0        

    ldr    x0, =HCR_VALUE
    msr    hcr_el2, x0

    ldr    x0, =SCR_VALUE
    msr    scr_el3, x0

    ldr    x0, =SPSR_VALUE
    msr    spsr_el3, x0

    adr    x0, el1_entry        
    msr    elr_el3, x0

    eret

el1_entry:
    // Cleans the .bss section, setting all values to 0 using memzero
    adr x0, bss_begin // adr calculates the address of a memory location to store it
    adr x1, bss_end
    // Subtracts the bss_begin and bss_end for size/length
    sub x1, x1, x0
    bl memzero

    mov sp, #LOW_MEMORY // Set the stack pointer. LOW_MEMORY = 4MB
    bl kernel_main // Pass execution to kernal_main
    b proc_hang // If we return for some reason, hang the program

proc_hang:
    wfe
    b proc_hang