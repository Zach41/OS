#define GLOBAL_VARIABLES_HERE

#include "const.h"
#include "type.h"
#include "hd.h"
#include "fs.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PUBLIC PROCESS        proc_table[NR_TASKS + NR_PROCS];

PUBLIC char           task_stack[STACK_SIZE_TOTAL];

PUBLIC TASK           task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "tty"},
					      {task_sys, STACK_SIZE_SYS, "sys_task"},
					      {task_hd, STACK_SIZE_HD, "task_hd"},
					      {task_fs, STACK_SIZE_FS, "task_fs"}};

PUBLIC TASK           user_proc_table[NR_PROCS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
						   {TestB, STACK_SIZE_TESTB, "TestB"},
						   {TestC, STACK_SIZE_TESTC, "TestC"}};

PUBLIC irq_handler    irq_table[NR_IRQ];

PUBLIC system_call    sys_call_table[NR_SYS_CALL] = {sys_write, sys_sendrec, sys_printx};

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
    
