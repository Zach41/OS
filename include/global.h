/* 和global.c一起使得全局变量只定义一次 */
#ifdef GLOBAL_VARIABLES_HERE
#undef  EXTERN
#define EXTERN
#endif

EXTERN int        disp_pos;
EXTERN u8         gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8         idt_ptr[6];
EXTERN GATE       idt[IDT_SIZE];

EXTERN PROCESS*   p_proc_ready;
EXTERN TSS        tss;

EXTERN int        k_reenter;

/* 当前控制台序号 */
EXTERN int        nr_current_console;

/* 系统时钟数 */
EXTERN int        ticks;

extern PROCESS            proc_table[];	/* 进程表 */
extern char               task_stack[];
extern TASK               task_table[];
extern TASK               user_proc_table[];
extern irq_handler        irq_table[];
extern system_call        sys_call_table[];
extern TTY                tty_table[];
extern CONSOLE            console_table[];
extern struct dev_drv_map dd_map[];


