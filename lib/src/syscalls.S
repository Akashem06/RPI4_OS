#include "syscalls.h"

.globl call_sys_write
.call_sys_write:
    mov w8, #SYS_WRITE_NUMBER
    svc #0
    ret

.globl call_sys_malloc
.call_sys_malloc:
    mov w8, #SYS_MALLOC_NUMBER
    svc #0
    ret

.globl call_sys_create_task
.call_sys_create_task:
    /* Save args for the child task */
    mov x10, x0 /* Func name */
    mov x11, x1 /* Args */
    mov x12, x2 /* Pointer to the new task's stack */

    /* Do the syscall */
    mov x0, x2
    mov w8, #SYS_CREATE_TASK_NUMBER
    svc #0

    cmp x0, #0
    beq thread_start
    ret
    
.globl call_sys_exit
.call_sys_exit:
    mov w8, #SYS_EXIT_NUMBER
    svc #0
    ret

thread_start:
    mov    x29, 0

    /* Pick the function arg and execute.  */
    mov    x0, x11
    blr    x10

    /* We are done, pass the return value through x0.  */
    mov    x8, #SYS_EXIT_NUMBER
    svc    0x0