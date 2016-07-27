#include "headers.h"
#include "stdio.h"

void Init() {
    char tty_name[] = "/dev_tty0";
    int fd_stdin  = open(tty_name, O_RDWR);
    assert(fd_stdin == 0);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == 1);
    int pid = getpid();
    printf("Init begins PID: %d\n", pid);

    untar("/cmd.tar");
    
    char* ttys[] = {"/dev_tty1", "/dev_tty2"};

    pid = fork();
    if (pid) {
	printf("Parent is running, child : %d\n", pid);
    } else {
	printf("Child is running, pid : %d\n", getpid());
	close(fd_stdin);
	close(fd_stdout);
	simple_shell("/dev_tty1");
	assert(0);
    }

    pid = fork();
    if (pid) {
    	printf("Parent is running, child : %d\n", pid);
    } else {
    	printf("Child is running, pid : %d\n", getpid());
    	close(fd_stdin);
    	close(fd_stdout);
    	simple_shell("/dev_tty2");
    	assert(0);
    }

    while (TRUE) {
	int s;
	int child = wait(&s);
	printf("child %d exited with status %d\n", child, s);
    }
    spin("Init Process");
}
/* 操作系统第一个进程的代码 */
void TestA() {
    spin("TestA");
}

/* 第二个进程的代码 */
void TestB() {

    spin("TestB");
}

void TestC() {
    int i=0;
    while (1) {
	/* printl("C"); */
	/* milli_delay(200); */
    }
}

/* 内核的代码 */
PUBLIC int kernel_main() {
    disp_str("--------Kernel Main Starts--------\n");

    TASK*    p_task;
    PROCESS* p_proc       = proc_table;
    char*    p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u8       privilege;
    u8       rpl;
    int      eflags;
    int      prio;
        
    for (int i=0; i<NR_TASKS + NR_PROCS; i++) {
	if (i >= NR_TASKS + NR_NATIVE_PROCS) {
	    p_proc -> p_flags = FREE_SLOT;
	    p_proc++;
	    continue;
	}
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
	    prio      = 15;
	}

	strcpy(p_proc -> p_name, p_task -> name);
	p_proc -> pid = i;
	p_proc -> p_parent = NO_TASK;
	/* p_proc -> ldt_sel = selector_ldt; */

	if (strcmp(p_task -> name, "INIT") != 0) {
	    /* 不是Init进程 */
	    memcpy(&p_proc -> ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
	    memcpy(&p_proc -> ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
	    p_proc -> ldts[0].attr1 = DA_C | privilege << 5;
	    p_proc -> ldts[1].attr1 = DA_DRW | privilege << 5;
	} else {
	    /* Init 进程 */
	    u32 k_base, k_limit;
	    int ret = get_kernel_map(&k_base, &k_limit);
	    assert(ret == 0);

	    init_descriptor(&p_proc -> ldts[0],
			    0,
			    (k_base + k_limit) >> LIMIT_4K_SHIFT,
			    DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);
	    init_descriptor(&p_proc -> ldts[1],
			    0,
			    (k_base + k_limit) >> LIMIT_4K_SHIFT,
			    DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
	    
	}

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
    }
    
    init_clock();
    /* init_keyboard(); */
    
    k_reenter = 0;

    ticks = 0;

    p_proc_ready = proc_table;
    
    restart();

    while (TRUE) {}
}

PUBLIC void simple_shell(const char* tty_name) {
    int fd_stdin  = open(tty_name, O_RDWR);
    assert(fd_stdin == 0);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == 1);
    char rdbuf[128];
    char wbuf[128];
    
    while (TRUE) {
	sprintf(wbuf, "%s$ ", tty_name);
	write(fd_stdout, wbuf, strlen(wbuf));
        int r = read(fd_stdin, rdbuf, 100);
        rdbuf[r] = 0;

        char* arg_stack[PROC_ORIGIN_STACK];
        char* str;
        char* p = rdbuf;
        int word = 0;
        int argc = 0;
        char ch;
        do {
            ch = *p;
            if (*p != ' ' && *p && !word) {
                /* word开头 */
                str = p;
                word = 1;
            }
            if ((*p == ' ' || *p == 0) && word) {
                /* word结尾 */
                word = 0;
                arg_stack[argc++] = str;
                *p = 0;
            }
            p++;
        } while(ch);
        arg_stack[argc] = 0;

        int fd = open(arg_stack[0], O_RDWR);
        if (fd == -1) {
            printf("Failed to open %s, no such file.\n", arg_stack[0]);
        } else {
            close(fd);
            int pid = fork();

            if (pid) {
                int status;
                wait(&status);
                /* printf("PROC %s exited.\n", arg_stack[0]); */
            } else {
                execv(arg_stack[0], arg_stack);
            }
        }
    }
    close(fd_stdin);
    close(fd_stdout);
}
