// Setup vector table
#include "entry.h"

// Saves the state of the cpu registers (x0 to x30) onto the stack
// stp stores a pair of two registers into memory
// 1 register is 64 bits (8 bytes), so each stp increments by 16 bytes
.macro kernel_entry
    sub sp, sp, #S_FRAME_SIZE

    // Save x0,x1 first since we'll use them as scratch registers
    stp x0, x1, [sp, #16 * 0]
    
    mrs x0, elr_el1
    mrs x1, spsr_el1
    stp x30, x0, [sp, #16 * 15] 
    str x1, [sp, #16 * 16]
    
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
.endm

.macro kernel_exit
    // Restore system registers first using x0,x1 as scratch
    ldr x1, [sp, #16 * 16]
    ldp x30, x0, [sp, #16 * 15] 
    
    msr elr_el1, x0
    msr spsr_el1, x1

    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    
    add sp, sp, #S_FRAME_SIZE
    eret
.endm

.globl dump_registers
dump_registers:
    stp     x29, x30, [sp, #-16]! 
    mov     x29, sp

    mov     x0, x7
	mov		x1, #7
    bl      print_register

    mov     x0, x8
	mov		x1, #8
    bl      print_register

    mov     x0, x8
	mov		x1, #8
    bl      print_register

    mov     x0, x9
	mov		x1, #9
    bl      print_register

    mov     x0, x19
	mov		x1, #19
    bl      print_register
    
    mov     x0, x20
	mov		x1, #20
    bl      print_register
    
    mov     x0, x21
	mov		x1, #21
    bl      print_register
    
    mov     x0, x22
	mov		x1, #22
    bl      print_register
    
    mov     x0, x23
	mov		x1, #23
    bl      print_register
    
    mov     x0, x24
	mov		x1, #24
    bl      print_register
    
    mov     x0, x25
	mov		x1, #25
    bl      print_register
    
    mov     x0, x26
	mov		x1, #26
    bl      print_register
    
    mov     x0, x27
	mov		x1, #27
    bl      print_register
    
    mov     x0, x28
	mov		x1, #28
    bl      print_register
    
    mov     x0, x29
	mov		x1, #29
    bl      print_register
    
    mov     x0, x30 
	mov		x1, #30
    bl      print_register

    ldp     x29, x30, [sp], #16
    ret

.macro handle_invalid_entry type
    kernel_entry
    mov x0, #\type
    mrs x1, esr_el1
    mrs x2, elr_el1
	mrs x3, far_el1
	mov x4, sp
    bl show_invalid_entry_message
	bl dump_registers
    b  err_hang
.endm

.macro ventry label
.align 7
    b \label
.endm

.align 11
.globl vectors
vectors:
    ventry sync_invalid_el1t
    ventry irq_invalid_el1t
    ventry fiq_invalid_el1t
    ventry error_invalid_el1t

    ventry sync_invalid_el1h
    ventry handle_el1_irq
    ventry fiq_invalid_el1h
    ventry error_invalid_el1h

    ventry sync_invalid_el0_64
    ventry irq_invalid_el0_64
    ventry fiq_invalid_el0_64
    ventry error_invalid_el0_64

    ventry sync_invalid_el0_32
    ventry irq_invalid_el0_32 
    ventry fiq_invalid_el0_32
    ventry error_invalid_el0_32

sync_invalid_el1t:
	handle_invalid_entry  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry  SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_invalid_entry  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  ERROR_INVALID_EL0_32

handle_el1_irq:
    kernel_entry
    bl handle_irq
    kernel_exit

.section .text
.align 4
.globl cpu_new_task
cpu_new_task:
    // Save frame pointer and link register
    stp     x29, x30, [sp, #-16]!
    mov     x29, sp

    
    // Move argument and call function
    bl      preempt_enable
    mov     x0, x20
    blr     x19
    
    // Restore frame and link register
    ldp     x29, x30, [sp], #16
    ret

.globl err_hang
err_hang: b err_hang
