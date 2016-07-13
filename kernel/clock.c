/* 时钟中断处理函数 */
#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC void clock_handler(int irq) {
    /* 时钟数加1 */
    ticks++;
    /* 当前进程的ticks减1 */
    p_proc_ready -> ticks--;
    if (k_reenter != 0) {
	return;
    }

    if (p_proc_ready -> ticks > 0) {
	return;			/* 知道ticks为0，才允许抢占 */
    }
    schedule();
}
