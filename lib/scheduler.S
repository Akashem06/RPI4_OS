#include "scheduler.h"

.section .text
.align 4
.globl cpu_context_switch
cpu_context_switch:

	mrs x10, daif
    mov x9, sp 

	mov	x11, #CPU_CONTEXT_OFFSET
	add	x8, x0, x11
	// Save context into previous task
	stp	x19, x20, [x8], #16
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	x30, [x8]
	// Restore context from next task
	add	x8, x1, x11
	ldp	x19, x20, [x8], #16
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x9, [x8], #16
	ldr	x30, [x8]
    msr daif, x10  
	mov	sp, x9
	msr daifclr, #2
	ret

.globl get_cpu_new_task_addr
get_cpu_new_task_addr:
    adr x0, cpu_new_task
    ret
