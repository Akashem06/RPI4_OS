#pragma once

#define NUM_SYSCALLS            4U

#define SYS_WRITE_NUMBER        0U
#define SYS_MALLOC_NUMBER       1U
#define SYS_CREATE_TASK_NUMBER  2U
#define SYS_EXIT_NUMBER         3U

#ifndef __ASSEMBLER__

void call_sys_write(void);
void call_sys_malloc(void);
void call_sys_create_task(void);
void call_sys_exit(void);

#endif
