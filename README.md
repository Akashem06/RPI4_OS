Great resource: https://github.com/s-matyukevich/raspberry-pi-os/tree/master
This got me started on baremetal dev on the RPI

Educational project!
Adapted for RPI3/RPI4

Includes a custom bluetooth host stack written by me. Can be found in this repo: https://github.com/Akashem06/RPI_Bluetooth

# Learnings
Major debug learning I had throughout this project. I think these are the most valuable experiences.

## OS Scheduler hard faulting after updating the link register during context switch (Return address, so it will be the next line of code to run).

#### What helped:
Register dumping when I hard fault. Helpful to see x19-x28 + Stack pointer + program counter.
Lower registers are mostly scratch registers that aren't too helpful :(
Dumping these also helped:
- esr_el1: This register provides the error code for hard faulting (The cause of exception).
  - Bits [31:26] - Exception Class (EC): Indicates type of violation(e.g., data abort, instruction abort, illegal execution).
  - Bits [24:0] - Instruction-Specific Syndrome (ISS): Extra information about the exception, such as details about the memory access that caused the fault.
- elr_el1: This register holds the return address after completing the exception. Hence e**lr**, its a link register.
- far_el1: Holds the faulting Virtual Address for all sync instructions, abort exceptions, data abort exceptions, and PC alignment fault exceptions. Since I did not have an MMU setup during this process, this was 0.

#### Debug Session:
Debug logs:
ERROR CAUGHT: SYNC_INVALID_EL1h - 4. ESR: 33554432 Address: 11968
Fault addr_reg: 0, stack pointer: 4202240
REG_NUMBER: 7, VALUE: 588976452
REG_NUMBER: 8, VALUE: 4198496
REG_NUMBER: 8, VALUE: 4198496
REG_NUMBER: 9, VALUE: 4202496
REG_NUMBER: 19, VALUE: 548144
REG_NUMBER: 20, VALUE: 0
REG_NUMBER: 21, VALUE: 0
REG_NUMBER: 22, VALUE: 0
REG_NUMBER: 23, VALUE: 0
REG_NUMBER: 24, VALUE: 0
REG_NUMBER: 25, VALUE: 0
REG_NUMBER: 26, VALUE: 0
REG_NUMBER: 27, VALUE: 0
REG_NUMBER: 28, VALUE: 0
REG_NUMBER: 29, VALUE: 4202224
REG_NUMBER: 30, VALUE: 530620

My custom log library doesn't support hex at the time of this fix so... Had to deal with decimal values.
ESR = 0x2000000. So the EC = 0x20: This means Instruction Abort in lower EL.
The fault occurred because of an instruction abort while in a lower Exception Level (EL1 in this case).

SYNC_INVALID_EL1h is the most helpful. This exception is thrown when a synchronous error occurs in EL1.

SYNC = Exception is related to the instruction flow. Fault caused by executing an invalid instruction, an access to an unmapped memory location, etc. Synchronous exceptions happen in direct response to program execution, unlike asynchronous exceptions (like interrupts), which occur independently of the current instruction flow.

INVALID = I messed up :(

EL1h = Happened in EL1 (Which I set in the boot process). the 'h' means we are in handler mode.

Ok so it seems to be an invalid memory access! But the addresses are all correct, I checked with debug statements.
The exception link register (elr_el1) is holding the correct function address, but it does seem fishy.

#### My misconception:
I was using relative function address rather than the absolute.

The stack pointer is set to LOW_MEMORY during bootup. This is at 4MB. When accessing the location of our new task's function, it is relative to our stack pointer. This would be fine in the case where the function is located in a memory region that can be reached with a relative address, such as within a small offset range from the current position of the stack pointer.

However, in cases where the function address is far from the stack pointer (e.g., if it's located in a different section of memory such as .text or .rodata), using a relative address would cause incorrect memory access, resulting in an instruction abort (as in this case). This is because the relative address calculation wouldn't be sufficient to access the correct memory location for the function.

By default, relative addressing in ARM assembly (with LDR) calculates offsets relative to the program counter or the current instruction, which can be problematic when jumping between different memory regions.

#### The fix:
Fix 1 (Implemented): Use ADR rather than LDR to load the address of the function. This fetches the absolute memory address.
Fix 2: Add LOW_MEMORY to the obtained memory address using LDR and ADD in assembly.

#### Key concepts:
Exception handling.
Memory access.
Context switching.

## ROData not being found at the correct memory location
I forgot to set the location counter in the linkerscript. This is essential. The kernel load address for AArch64 is at 
0x80000. This means everything compiled and flashed is at an offset + 0x80000. After adding this to the linkerscript
I was able to correctly read the rodata. 
