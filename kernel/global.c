#define GLOBAL_VARIABLES_HERE

#include "headers.h"

PUBLIC PROCESS        proc_table[NR_TASKS + NR_PROCS];

PUBLIC char           task_stack[STACK_SIZE_TOTAL];

PUBLIC TASK           task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "tty"},
					      {task_sys, STACK_SIZE_SYS, "sys_task"},
					      {task_hd, STACK_SIZE_HD, "task_hd"},
					      {task_fs, STACK_SIZE_FS, "task_fs"},
					      {task_mm, STACK_SIZE_MM, "task_mm"}};

PUBLIC TASK           user_proc_table[NR_PROCS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
						   {TestB, STACK_SIZE_TESTB, "TestB"},
						   {TestC, STACK_SIZE_TESTC, "TestC"},
						   {Init,  STACK_SIZE_INIT,  "INIT"}};

PUBLIC irq_handler    irq_table[NR_IRQ];

PUBLIC system_call    sys_call_table[NR_SYS_CALL] = {sys_sendrec, sys_printx};

/* tty */
PUBLIC TTY            tty_table[NR_CONSOLE];
PUBLIC CONSOLE        console_table[NR_CONSOLE];

PUBLIC struct dev_drv_map dd_map[] = {
    {INVALID_DRIVER},
    {INVALID_DRIVER},
    {INVALID_DRIVER},
    {TASK_HD},
    {TASK_TTY},
    {INVALID_DRIVER}
};

/* 内存地址6MB~7MB为文件系统缓冲，一个文件最大为1MB */
PUBLIC u8*          fsbuf      = (u8*)0x600000;
PUBLIC const int    FSBUF_SIZE = 0x100000;

PUBLIC u8*          mmbuf      = (u8*)0x700000;
PUBLIC const int    MMBUF_SIZE = 0x100000;
    
