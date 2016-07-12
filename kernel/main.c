#include "const.h"
#include "type.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"


/* 操作系统第一个进程的代码 */
void TestA() {
    while(1) {
	disp_str("A");
	disp_str(" ");
	delay(1);
    }
}

/* 第二个进程的代码 */
void TestB() {
    while(1) {
	disp_str("B");
	disp_str(" ");
	delay(1);
    }
}
/* 内核的代码 */
PUBLIC int kernel_main() {
    int text_color = 0x74;

    disp_str("--------Kernel Main Starts--------\n");

    PROCESS* p_proc = proc_table; /* p_proc指向进程表第一个进程控制块 */

    p_proc -> ldt_sel = SELECTOR_LDT_FIRST;
    /* 拷贝描述符到本地 */
    memcpy(&p_proc -> ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
    memcpy(&p_proc -> ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
    p_proc -> ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
    p_proc -> ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

    /* 设置寄存器 */
    p_proc -> regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
    p_proc -> regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
    p_proc -> regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
    p_proc -> regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
    p_proc -> regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
    p_proc -> regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK ) | RPL_TASK;
    p_proc -> regs.eip = (u32)TestA;
    p_proc -> regs.esp = (u32)task_stack + STACK_SIZE_TOTAL;
    p_proc -> regs.eflags = 0x1202;

    p_proc_ready = p_proc;

    k_reenter = -1;

    restart();
    
    while (1){}
}
