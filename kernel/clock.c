/* 时钟中断处理函数 */
#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC void clock_handler(int irq) {
    disp_str("#");
    p_proc_ready++;

    if (p_proc_ready >= proc_table + NR_TASKS) {
	p_proc_ready = proc_table;
    }
}
