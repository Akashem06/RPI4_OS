.section ".text.boot"
.globl _start
.globl __code_start

// System Register Definitions
#define SCTLR_RESERVED              ((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11))
#define SCTLR_EE_LITTLE_ENDIAN      (0 << 25)
#define SCTLR_I_CACHE_DISABLED      (0 << 12)
#define SCTLR_D_CACHE_DISABLED      (0 << 2)
#define SCTLR_MMU_DISABLED          (0 << 0)
#define SCTLR_VALUE_MMU_DISABLED    (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

#define HCR_RW                      (1 << 31)

#define SCR_RESERVED                (3 << 4)
#define SCR_RW                      (1 << 10)
#define SCR_NS                      (1 << 0)
#define SCR_VALUE                   (SCR_RESERVED | SCR_RW | SCR_NS)

#define SPSR_MASK_ALL               (7 << 6)
#define SPSR_EL1h                   (5 << 0)
#define SPSR_VALUE                  (SPSR_MASK_ALL | SPSR_EL1h)
#define LOW_MEMORY                  0x400000

__code_start:
_start:
    // Read core ID and ensure we're on primary core (core 0)
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, master
    b       proc_hang

master:
    // Check current exception level
    mrs     x0, CurrentEL
    lsr     x0, x0, #2

    cmp     x0, #3
    beq     el3_setup

    cmp     x0, #2
    beq     el2_setup

    cmp     x0, #1
    beq     el1_entry

    b       proc_hang

el3_setup:
    // Configure EL2
    ldr     x0, =HCR_RW
    msr     hcr_el2, x0

    // Configure SCR_EL3
    ldr     x0, =SCR_VALUE
    msr     scr_el3, x0

    // Prepare SPSR for EL1 entry
    ldr     x0, =SPSR_VALUE
    msr     spsr_el3, x0

    // Set return address to el1_entry
    adr     x0, el1_entry
    msr     elr_el3, x0

    // Return to EL1
    eret

el2_setup:
    // Configure HCR_EL2 for 64-bit EL1
    ldr     x0, =HCR_RW
    msr     hcr_el2, x0

    // Prepare SPSR for EL1 entry
    ldr     x0, =SPSR_VALUE
    msr     spsr_el2, x0

    // Set return address to el1_entry
    adr     x0, el1_entry
    msr     elr_el2, x0

    dsb sy
    isb

    // Return to EL1
    eret

el1_entry:
    mrs     x0, CurrentEL
    lsr     x0, x0, #2

    cmp     x0, #1
    bne     proc_hang

    ldr     x0, =SCTLR_VALUE_MMU_DISABLED
    msr     sctlr_el1, x0

    msr     SPSel, #1

    ldr     x0, =LOW_MEMORY
    mov     sp, x0

    dsb sy
    isb

    // Zero BSS section
    adrp    x0, __bss_begin
    add     x0, x0, :lo12:__bss_begin

    adrp    x1, __bss_end
    add     x1, x1, :lo12:__bss_end
    
    sub     x2, x1, x0
    lsr     x2, x2, #3
    cbz     x2, skip_bss_zero

bss_zero_loop:
    str     xzr, [x0], #8
    subs    x2, x2, #1
    bne     bss_zero_loop

skip_bss_zero:
    // Call kernel main
    bl      kernel_main

proc_hang:
    wfe
    b       proc_hang

.section ".text"
.globl __text_start
__text_start: