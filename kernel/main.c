#include "headers.h"
#include "stdio.h"

/* 操作系统第一个进程的代码 */
void TestA() {
    /* char *filename = "hello"; */
    /* const char bufw[] = "hello, file system"; */
    /* const int rd_bytes = 11; */
    /* char  bufr[rd_bytes+1]; */

    /* int fd = open(filename, O_CREAT | O_RDWR); */
    /* assert(fd != -1); */
    /* printf("FD: %d\n", fd); */

    /* /\* write *\/ */
    /* int n = writef(fd, bufw, strlen(bufw)); */
    /* assert(n == strlen(bufw)); */

    /* close(fd); */

    /* fd = open(filename, O_RDWR); */

    /* /\* read *\/ */
    /* n = readf(fd, bufr, rd_bytes); */
    /* assert(n == rd_bytes); */
    /* bufr[n] = 0; */
    /* printf("%d bytes read. [%s]\n", n, bufr); */
    /* close(fd); */

    int fd;

    /* 测试文件删除 */
    char* files[] = {"/foo", "bar", "/bbb"};
    for (int i=0; i<3; i++) {
	fd = open(files[i], O_CREAT | O_RDWR);
	printl("create file: %s, fd: %d\n", files[i], fd);
	close(fd);
    }

    /* char* files_to_delete[] = {"/foo", "/bar", "dev_tty0"}; */

    /* for (int i=0; i<3; i++) { */
    /* 	if (unlink(files_to_delete[i]) == 0) { */
    /* 	    printl("File %s deleted.\n", files_to_delete[i]); */
    /* 	} else { */
    /* 	    printl("Failed to delete file %s.\n", files_to_delete[i]); */
    /* 	} */
    /* } */
    spin("TestA");
}

/* 第二个进程的代码 */
void TestB() {
    int i=0;
    while(1) {
	/* printf("B"); */
	milli_delay(200);
    }
}

void TestC() {
    int i=0;
    /* MESSAGE drive_msg; */
    /* drive_msg.type = DEV_OPEN; */

    /* send_recv(BOTH, TASK_HD, &drive_msg); */

    /* spin("Drive Message"); */
    while (1) {
	/* printl("C"); */
	milli_delay(200);
    }
}

/* 内核的代码 */
PUBLIC int kernel_main() {
    disp_str("--------Kernel Main Starts--------\n");

    TASK*    p_task;
    PROCESS* p_proc       = proc_table;
    char*    p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16      selector_ldt = SELECTOR_LDT_FIRST;
    u8       privilege;
    u8       rpl;
    int      eflags;
    int      prio;
        
    for (int i=0; i<NR_TASKS + NR_PROCS; i++) {
	if (i < NR_TASKS) {
	    /* 任务 */
	    p_task    = task_table + i;
	    privilege = PRIVILEGE_TASK;
	    rpl       = RPL_TASK;
	    eflags    = 0x1202;
	    prio      = 15;
	} else {
	    /* 用户进程 */
	    p_task    = user_proc_table + (i - NR_TASKS);
	    privilege = PRIVILEGE_USER;
	    rpl       = RPL_USER;
	    eflags    = 0x202;
	    prio      = 5;
	}
	strcpy(p_proc -> p_name, p_task -> name);
	p_proc -> pid = i;
	p_proc -> ldt_sel = selector_ldt;

	memcpy(&p_proc -> ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
	memcpy(&p_proc -> ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
	p_proc -> ldts[0].attr1 = DA_C | privilege << 5;
	p_proc -> ldts[1].attr1 = DA_DRW | privilege << 5;
	/* 设置寄存器 */
	// cs指向LDT的第一个表项
	p_proc -> regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | rpl | SA_TIL;
	// ds指向LDT的第二个表项
	p_proc -> regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | rpl | SA_TIL;
	p_proc -> regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | rpl | SA_TIL;
	p_proc -> regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | rpl | SA_TIL;
	p_proc -> regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | rpl | SA_TIL;
	/* 显存段的选择子是全局的，但是要修改任务的优先级 */
	p_proc -> regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

	p_proc -> regs.eip     = (u32)p_task -> initial_eip;
	p_proc -> regs.esp     = (u32)p_task_stack;
	p_proc -> regs.eflags  = eflags;
	p_proc -> nr_tty       = 0;
	p_proc -> p_flags      = 0;
	p_proc -> p_msg        = 0;
	p_proc -> p_sendto     = NO_TASK;
	p_proc -> p_recvfrom   = NO_TASK;
	p_proc -> has_int_msg  = 0;
	p_proc -> q_sending    = 0;
	p_proc -> next_sending = 0;

	p_proc -> priority     = prio;
	p_proc -> ticks        = prio;

	for (int j=0; j<NR_FILES; j++)
	    p_proc -> filp[j] = 0;

	/* 堆栈由高往地处生长 */
	p_task_stack -= p_task -> stacksize;
	p_proc++;
	p_task++;

	selector_ldt += 8;
    }

        /* 测试代码 */
    /* proc_table[0].priority = 20; */
    /* proc_table[0].ticks    = 20; */
    /* proc_table[1].priority = 20; */
    /* proc_table[1].ticks    = 20; */
    /* proc_table[2].priority = 20; */
    /* proc_table[2].ticks    = 20; */
    /* proc_table[3].priority = proc_table[3].ticks = 20; */
    proc_table[TASK_HD].nr_tty        = 2;
    proc_table[TASK_FS].nr_tty        = 0;
    proc_table[NR_TASKS + 0].nr_tty   = 0;
    proc_table[NR_TASKS + 1].nr_tty   = 1;
    proc_table[NR_TASKS + 2].nr_tty   = 1;
    
    init_clock();
    /* init_keyboard(); */
    
    k_reenter = 0;

    ticks = 0;

    p_proc_ready = proc_table;
    
    restart();

    while (TRUE) {}
}
