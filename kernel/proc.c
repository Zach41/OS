#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

/* 系统调用的函数 */
PUBLIC int sys_get_ticks() {
    /* 返回当前的时钟数 */
    return ticks;
}


/* 进程调度函数 */
PUBLIC void schedule() {
    /* 优先级最大的优先执行 */
    PROCESS* p;
    int largest_ticks = 0;

    while (largest_ticks == 0) {
    	for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS ; p++) {
    	    if (p -> ticks > largest_ticks) {
    		p_proc_ready = p;
    		largest_ticks = p -> ticks;
	    }
	}
        if (largest_ticks == 0) {
       	/* 全部为0, 那么重新赋值 */
    	    for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
    		p -> ticks = p -> priority;
    	    }
        }
    }
}
