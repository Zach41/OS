#include "const.h"
#include "type.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"


/* 操作系统第一个进程的代码 */
void TestA() {
    int i=0;
    while(1) {
	disp_str("A");
	disp_str(" ");
	disp_int(i++);
	delay(1);
    }
}

/* 第二个进程的代码 */
void TestB() {
    int i=0;
    while(1) {
	disp_str("B");
	disp_str(" ");
	disp_int(i++);
	delay(1);
    }
}

void TestC() {
    int i=0;
    while (1) {
	disp_str("C");
	disp_str(" ");
	disp_int(i++);
	delay(1);
    }
}

/* 内核的代码 */
PUBLIC int kernel_main() {
    int text_color = 0x74;

    disp_color_str("--------Kernel Main Starts--------\n", text_color);

    /* PROCESS* p_proc = proc_table; /\* p_proc指向进程表第一个进程控制块 *\/ */

    /* p_proc -> ldt_sel = SELECTOR_LDT_FIRST; */
    /* /\* 拷贝描述符到本地 *\/ */
    /* memcpy(&p_proc -> ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR)); */
    /* memcpy(&p_proc -> ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR)); */
    /* p_proc -> ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5; */
    /* p_proc -> ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5; */

    /* /\* 设置寄存器 *\/ */
    /* p_proc -> regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; */
    /* p_proc -> regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; */
    /* p_proc -> regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; */
    /* p_proc -> regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; */
    /* p_proc -> regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; */
    /* p_proc -> regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK ) | RPL_TASK; */
    /* p_proc -> regs.eip = (u32)TestA; */
    /* p_proc -> regs.esp = (u32)task_stack + STACK_SIZE_TOTAL; */
    /* p_proc -> regs.eflags = 0x1202; */

    /* p_proc_ready = p_proc; */

    TASK*    p_task       = task_table;
    PROCESS* p_proc       = proc_table;
    char*    p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16      selector_ldt = SELECTOR_LDT_FIRST;

    for (int i=0; i<NR_TASKS; i++) {
	strcpy(p_proc -> p_name, p_task -> name);
	p_proc -> pid = i;
	
	p_proc -> ldt_sel = selector_ldt;

	memcpy(&p_proc -> ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
	memcpy(&p_proc -> ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
	p_proc -> ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
	p_proc -> ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
	/* 设置寄存器 */
	// cs指向LDT的第一个表项
	p_proc -> regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | RPL_TASK | SA_TIL;
	// ds指向LDT的第二个表项
	p_proc -> regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | RPL_TASK | SA_TIL;
	p_proc -> regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | RPL_TASK | SA_TIL;
	p_proc -> regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | RPL_TASK | SA_TIL;
	p_proc -> regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | RPL_TASK | SA_TIL;
	/* 显存段的选择子是全局的，但是要修改任务的优先级 */
	p_proc -> regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

	p_proc -> regs.eip    = (u32)p_task -> initial_eip;
	p_proc -> regs.esp    = (u32)p_task_stack;
	p_proc -> regs.eflags = 0x1202; /* IF=1, IOPL=1 */

	/* 堆栈由高往地处生长 */
	p_task_stack -= p_task -> stacksize;
	p_proc++;
	p_task++;

	selector_ldt += 8;
    }

    k_reenter = 0;

    p_proc_ready = proc_table;
    
    restart();
    
    while (1){}
}
