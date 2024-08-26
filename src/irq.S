// D13.2.136

// Set up vectors
.globl irq_init_vectors
irq_init_vectors:
    adr x0, vectors
    msr vbar_el1, x0
    ret

// C5.2.2
// DAIF Interrupt mask bits
// D for debug (watchpoint/breakpiont masks)
// A for Serror mask bit
// I for IRQ mask bit
// F for FIQ mask bit
.globl irq_enable
irq_enable:
    msr daifclr, #2 // Clears bit 2, enablin IRQs
    ret

.globl irq_disable
irq_disable:
    msr daifset, #2 // Sets bit 2, disabling IRQs
    ret