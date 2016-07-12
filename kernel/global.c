#define GLOBAL_VARIABLES_HERE

#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC PROCESS        proc_table[NR_TASKS];

PUBLIC char           task_stack[STACK_SIZE_TOTAL];
