#define GLOBAL_VARIABLES_HERE

#include "const.h"
#include "type.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC PROCESS        proc_table[NR_TASKS];

PUBLIC char           task_stack[STACK_SIZE_TOTAL];

PUBLIC TASK           task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
					      {TestB, STACK_SIZE_TESTB, "TestB"},
					      {TestC, STACK_SIZE_TESTC, "TestC"},
					      {task_tty, STACK_SIZE_TTY, "tty"}};

PUBLIC irq_handler    irq_table[NR_IRQ];

PUBLIC system_call    sys_call_table[NR_SYS_CALL] = {sys_get_ticks};

/* tty */
PUBLIC TTY            tty_table[NR_CONSOLE];
PUBLIC CONSOLE        console_table[NR_CONSOLE];
