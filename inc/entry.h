#pragma once

// D1.10.2 Arm ref

/// Interrupt occurs, the execution goes to the exception vector.
// Exception is in vector table, that is specified at VBAR
// Vector Base Address Register. Exception is either:

// Synchronous exception - Result of specific instructions
// Examples: Divide by 0, memory access violation. CPU encounters errors

// SError (System Error) = System level errors triggered by hardware or system events

// IRQ - Asynchronous exceptions triggered by hardware changes. Timer interrupts, I/O interrupt

// FIQ (Fast Interrupt Request) - Higher priority IRQs with minimal latency

#define SYNC_INVALID_EL1t       0
#define IRQ_INVALID_EL1t        1
#define FIQ_INVALID_EL1t        2
#define ERROR_INVALID_EL1t      3

#define SYNC_INVALID_EL1h       4
#define IRQ_INVALID_EL1h        5
#define FIQ_INVALID_EL1h        6
#define ERROR_INVALID_EL1h      7

#define SYNC_INVALID_EL0_64     8
#define IRQ_INVALID_EL0_64      9
#define FIQ_INVALID_EL0_64      10
#define ERROR_INVALID_EL0_64    11

#define SYNC_INVALID_EL0_32     12
#define IRQ_INVALID_EL0_32      13
#define FIQ_INVALID_EL0_32      14
#define ERROR_INVALID_EL0_32    15

#define S_FRAME_SIZE            256

#ifndef __ASSEMBLER__
void cpu_new_task(void);
#endif
