#pragma once

#define NUM_SYSCALLS 4

#define SYS_WRITE_NUMBER 0
#define SYS_MALLOC_NUMBER 1
#define SYS_CREATE_TASK_NUMBER 2
#define SYS_EXIT_NUMBER 3

#ifndef __ASSEMBLER__

void call_sys_write(void);
void call_sys_malloc(void);
void call_sys_create_task(void);
void call_sys_exit(void);

#endif
