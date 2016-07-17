# 进程

进程是运行着的程序的一个实例。进程有自己的代码、堆栈和数据，它有自己的目标和功能，同时受控于进程调度模块。一般的操作系统同时运行着多个进程，各个进程完成各自的工作，互不影响。

## 进程控制块

当进程调度模块调度一个进程执行时，它必须有能力让进程恢复到上一次被终止的状态。所以我们有必要新建一个数据结构来保存进程的运行状态，即各个寄存器的值。这个数据结构就是`进程控制块(PCB)`。结构表示如下：

```C
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
}PROCESS;
```

每一个数据属性都比较好理解，至于为什么寄存器堆栈是这样的，这跟中断的时候寄存器压栈以及从内核恢复到进程(ring0 -> ring1)运行时堆栈的内容有关，具体可以参考书本`P51`和`P176`。需要注意的一点是在进程被中断的时候，堆栈指针的变换。

1. 进程没有被中断，esp指向进程堆栈的栈顶
2. 进程被切换，寄存器此时要被保存到进程控制块，此时esp指向`s_stackframe`的最高地址
3. 内核中断处理，此时esp指向内核堆栈。

三次的堆栈切换不可以混淆，以免破坏掉堆栈的数据。

由于我们的进程运行在ring1，这就涉及到特权级切换时堆栈的切换，除了进程控制块，我们还需要一个TSS，TSS中的ring0的ESP和SS便是进程跳入ring0时内核的堆栈寄存器。如何设置可以参照书本P180。

## 时钟中断处理

时钟中断处理和进程调度总是息息相关的，每一个时钟中断到来的时候，操作系统就要判断当前进程是否应该被调度，如果是，那么调度程序就被启动。

在中断处理的时候就要考虑堆栈的切换和中断重入的问题了。这里直接给出代码：


```NASM
%macro	hwint_master	1
	call	save
	;; save之后在内核堆栈当中
	in 	al, INT_M_CTLMASK
	or 	al, (1 << %1)	;　屏蔽当前中断
	out	INT_M_CTLMASK, al

	mov	al, EOI
	out 	INT_M_CTL, al

	sti

	push 	%1
	call	[irq_table + 4 * %1] ; 中断处理程序
	pop 	ecx
	cli

	in 	al, INT_M_CTLMASK
	and	al, ~(1 << %1)	;开启当前中断
	out	INT_M_CTLMASK, al
	
	...
	
save:	
	pushad
	push 	ds
	push 	es
	push 	fs
	push 	gs
	mov	dx, ss
	mov	ds, dx
	mov 	es, dx

	mov 	esi, esp	; eax <- 进程控制块起始地址

	inc	dword [k_reenter]
	cmp 	dword [k_reenter], 0
	jnz	.1

	mov	esp, StackTop	;　切换内核栈
	push 	restart
	jmp 	[esi + RETADR - P_STACKBASE] ; 返回到调用call的下一条指令

.1:
	push 	restart_reenter
	jmp 	[esi + RETADR - P_STACKBASE]
	
restart:
	;; 初始化进程运行
	mov	esp, [p_proc_ready]
	lldt	[esp+P_LDT_SEL]	; 加载LDT
	lea	eax, [esp + P_STACKTOP]
	mov 	dword [tss + TSS3_S_SP0], eax ; ring0的SP0
restart_reenter:
	dec 	dword [k_reenter]
	pop 	gs
	pop 	fs
	pop 	es
	pop 	ds
	popad

	add 	esp, 4		; 略过retaddr
	iretd
```


可以看到当中断发生的时候，首先保存当前进程的状态到进程控制块，然后利用`k_reenter`这个全局变量判断是否是中断重入(k_reenter > 0)，如果不是，那么切换进程，否则保持当前进程不变。这里需要注意的一个地方是在`save`函数的返回是直接`jmp [esi + RETADR - P_STACKBASE]`，这是因为`save`调用之前和调用之后的堆栈发生了变化，不能再简单的`ret`。

同时我们看到中断处理的部分在`call [irq_table + 4 * %1]`，`irq_table`从外部导入，由Ｃ语言编写。那么时钟中断的调用流程就是：

1. 时钟中断到来，中断向量里对应的处理函数被调用(hwint00)
2. hwint00调用irq_table内对应的函数
3. clock_hanlder(int)被调用。

其他的中断处理流程也是类似的。

这里给出`clock_handler`的代码

```C
PUBLIC void clock_handler(int irq) {
    /* 时钟数加1 */
    ticks++;
    /* 当前进程的ticks减1 */
    p_proc_ready -> ticks--;
    if (k_reenter != 0) {
	return;
    }

    if (p_proc_ready->ticks > 0) {
	return;			/* 知道ticks为0，才允许抢占 */
    }
    schedule();
}
```

## 进程调度模块

上一节有一个`schedule`函数的调用，这个就是我们的进程调度模块了。其实这个函数很简单，这里就不再多说，直接把代码贴出来:

```C
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
```

## 系统调用

在linux中，系统调用是通过中断实现的。用户程序通过调用特定函数，触发中断，陷入ring0的内核代码，内核处理完成后从中断返回到用户进程。在这里，我们也通过同样的方式来实现系统调用。

#### 1.

首先建立一个系统调用函数指针数组`sys_call_table`，每一个元素代码内核对特定的系统调用的处理逻辑

#### 2. 

新建一个`syscall.asm`，它的每一个函数代表用户进程可以调用的系统调用。

实现如下：

```NASM
	%include	"sconst.inc"

	_NR_get_ticks		equ	0
	_NR_write		equ 	1
	INT_VECTOR_SYS_CALL	equ	0x90

	global	get_ticks
	global	write

	bits 32
	[SECTION .text]

get_ticks:
	mov	eax, _NR_get_ticks ; eax内存入系统调用的函数索引
	int 	INT_VECTOR_SYS_CALL ; 调用中断
	ret

write:
	mov 	eax, _NR_write
	mov	ebx, [esp + 4]	; buf
	mov 	ecx, [esp + 8]	; len
	int     INT_VECTOR_SYS_CALL
	ret
```

可以看到系统调用的中断号是`0x90`，同时程序通过eax寄存器来获知用户进程调用的是哪一个系统调用。

#### 3.

最后我们需要在`kernel.asm`中编写中断向量号对应的处理函数并在内核初始化的时候初始化对应的中断门描述符。

```NASM
sys_call:	
	call	save
	push	dword [p_proc_ready]
	sti

	push 	ecx
	push 	ebx
	call	[sys_call_table + eax * 4]
	add	esp, 4*3
	mov	[esi + EAXREG - P_STACKBASE], eax ; 将系统调用返回值放入进程控制块的EAX寄存器，以便在进程恢复以后eax中装的是正确的返回值
	cli
	ret
```

```C
    /* 初始化系统调用的中断描述符 */
    init_idt_desc(INT_VECTOR_SYS_CALL, DA_386IGate, sys_call, PRIVILEGE_USER);
```

此后如果要新增一个系统调用，那么步骤就为：

1. 编写内核的处理逻辑，将函数指针加入`sys_call_table`
2. 在`syscall.asm`中编写对应的用户调用函数







