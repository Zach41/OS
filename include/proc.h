#ifndef _ZACH_PROC_H_
#define _ZACH_PROC_H_

/* stack frame */
typedef struct s_stackframe {
    u32    gs;
    u32    fs;
    u32    es;
    u32    ds;
    u32    edi;
    u32    esi;
    u32    ebp;
    u32    kernel_esp;
    u32    ebx;
    u32    edx;
    u32    ecx;
    u32    eax;
    u32    retaddr;
    u32    eip;
    u32    cs;
    u32    eflags;
    u32    esp;
    u32    ss;
}STACK_FRAME;

/* 进程控制块 */
typedef struct s_proc {
    STACK_FRAME regs;

    u16            ldt_sel;		/* 在GDT表中的该进程的LDT表的选择子 */
    DESCRIPTOR     ldts[LDT_SIZE];	/* 本地的LDT表 */

    int            ticks;	        /* 剩下的时钟数 */
    int            priority;		/* 优先级 */
    
    u32            pid;			/* 进程ID */
    char           p_name[16];		/* 进程名 */

    int            nr_tty;	        /* 进程对应的终端 */

    int            p_flags;	        /* 进程运行时，p_flags = 0 
					   SENDING: in sending message
					   RECEIVING: in receiving message
					 */

    MESSAGE        *p_msg;
    int            p_recvfrom;	        /* want message from proc id */
    int            p_sendto;            /* want message send to proc id */

    int            has_int_msg;	        /* 中断发生时，如果任务还没有准备好取处理，那么这个值为非负 */

    struct s_proc* q_sending;	        /* the queue of procs sending messages to this proc */
    struct s_proc* next_sending;        /* next proc in the sending queue */
}PROCESS;

/* 任务结构 */
typedef struct s_task {
    task_f  initial_eip;
    int     stacksize;
    char    name[32];
}TASK;

/* 任务的个数 */
#define NR_TASKS            2
#define NR_PROCS            3

#define STACK_SIZE_TESTA    0x8000
#define STACK_SIZE_TESTB    STACK_SIZE_TESTA
#define STACK_SIZE_TESTC    STACK_SIZE_TESTA
#define STACK_SIZE_TTY      STACK_SIZE_TESTA
#define STACK_SIZE_SYS      STACK_SIZE_TESTA
#define STACK_SIZE_TOTAL    (STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC + \
			     STACK_SIZE_TTY + STACK_SIZE_SYS)

#define proc2pid(x)    (x - proc_table)

#endif
