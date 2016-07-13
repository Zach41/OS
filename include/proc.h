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
    u32            pid;			/* 进程ID */
    char           p_name[16];		/* 进程名 */
}PROCESS;

/* 任务结构 */
typedef struct s_task {
    task_f  initial_eip;
    int     stacksize;
    char    name[32];
}TASK;

/* 任务的个数 */
#define NR_TASKS            3

#define STACK_SIZE_TESTA    0x8000
#define STACK_SIZE_TESTB    STACK_SIZE_TESTA
#define STACK_SIZE_TESTC    STACK_SIZE_TESTA
#define STACK_SIZE_TOTAL    (STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC)

