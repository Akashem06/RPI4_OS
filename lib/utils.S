.globl delay
delay:
    subs x0, x0, #1 // Subtracts 1 from the value in x0 (Function argument)
    bne delay       // Branch if not equal. So recursively call until x0 is 0
    ret

.globl put32
put32: 
    str w1, [x0] // Stores the 2nd argument (w1, which is 32 bits of x1) into memory at the address pointed to by x0
    ret

.globl get32
get32:
    ldr w0, [x0] // Loads value from memory (address in x0) into w0 (return register)
    ret

.globl get_el
get_el:
    mrs x0, CurrentEL // Move the value of the CurrentEL (Current Exception Level) special register into x0
    lsr x0, x0, #2    // Shift right by 2 bits (to ignore the two reserved-0 bits)
    ret
