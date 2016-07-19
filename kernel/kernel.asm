	%include "sconst.inc"

	extern 	gdt_ptr
	extern  idt_ptr
	extern 	disp_pos
	extern  p_proc_ready
	extern  tss
	extern	k_reenter
	extern	irq_table
	extern	sys_call_table

	extern 	cstart
	extern	exception_handler
	extern	disp_str
	extern	kernel_main
	extern	delay
	extern	clock_handler
	extern	spurious_irq	; 外部中断处理函数

[SECTION .data]
	color_int_msg	db	"^", 0
[SECTION .bss]
	StackSpace	resb	2*1024
	StackTop:

[SECTION .text]
	global 	_start

	global	restart		; 进程运行入口
	global 	sys_call	; 系统调用入口

	global 	divide_error
	global 	single_step_exception
	global 	nmi
	global	breakpoint_exception
	global	overflow
	global	bounds_check
	global	inval_opcode
	global	copr_not_available ; 无数学协处理器
	global 	double_fault
	global 	copr_seg_overrun ;　协处理器越界
	global	inval_tss
	global	segment_not_present
	global 	stack_exception
	global 	general_protection
	global	page_fault
	global	copr_error

	;; 外部中断
	global 	hwint00
	global 	hwint01
	global	hwint02
	global	hwint03
	global	hwint04
	global 	hwint05
	global	hwint06
	global	hwint07
	global 	hwint08
	global	hwint09
	global 	hwint10
	global 	hwint11
	global 	hwint12
	global 	hwint13
	global 	hwint14
	global 	hwint15

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

	ret
%endmacro

%macro	hwint_slave 	1
	call	save
	in	al, 	INT_S_CTLMASK
	or	al, 	(1 << (%1 - 8))
	out	INT_S_CTLMASK, 	al ; 屏蔽当前中断

	mov	al, EOI
	out	INT_M_CTL, 	al ; Master和Slave都要置EOI
	nop
	out	INT_S_CTL,	al ; 告诉CPU中断已经完成

	sti

	push	%1
	call	[irq_table + 4 * %1]
	pop 	ecx

	cli

	in 	al, INT_S_CTLMASK
	and 	al, ~(1 << (%1 - 8))
	out 	INT_S_CTLMASK,	al

	ret
%endmacro
	
_start:
	mov	esp, StackTop
	mov	dword [disp_pos], 0
	sgdt	[gdt_ptr]
	call	cstart
	lgdt	[gdt_ptr]

	;; 加载idt
	lidt	[idt_ptr]

	jmp	SELECTOR_KERNEL_CS:csinit

csinit:
	;; 加载tr
	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax
	jmp 	kernel_main

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

save:
	pushad
	push 	ds
	push 	es
	push 	fs
	push 	gs

	mov	esi, edx	; 保存edx，edx里保存了系统调用的参数
	
	mov	dx, ss
	mov	ds, dx
	mov 	es, dx

	mov	edx, esi	; 恢复edx

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
	

divide_error:
	push 	0xFFFFFFFF
	push 	0
	jmp 	exception
single_step_exception:
	push 	0xFFFFFFFF
	push 	1
	jmp 	exception
nmi:
	push	0xFFFFFFFF
	push 	2
	jmp 	exception
breakpoint_exception:
	push	0xFFFFFFFF
	push 	3
	jmp	exception
overflow:
	push 	0xFFFFFFFF
	push 	4
	jmp 	exception
bounds_check:
	push 	0xFFFFFFFF
	push 	5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF
	push	6
	jmp	exception
copr_not_available:
	push 	0xFFFFFFFF
	push 	7
	jmp	exception
double_fault:
	;; error code already in the stack
	push	8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF
	push	9
	jmp	exception
inval_tss:
	push	10
	jmp	exception
segment_not_present:
	push 	11
	jmp	exception
stack_exception:
	push	12
	jmp 	exception
general_protection:
	push	13
	jmp 	exception
page_fault:
	push	14
	jmp 	exception
copr_error:
	push	0xFFFFFFFF
	push	16
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4*2
	hlt

	;; 外部中断
ALIGN	16
hwint00:
	hwint_master	0
	
ALIGN	16
hwint01:
	hwint_master	1

ALIGN 	16	
hwint02:
	hwint_master	2	; cascade

ALIGN	16
hwint03:
	hwint_master	3	; second serial

ALIGN	16	
hwint04:
	hwint_master	4	; first serial

ALIGN	16	
hwint05:
	hwint_master	5	; XT winchester

ALIGN	16	
hwint06:
	hwint_master	6	; floppy

ALIGN	16	
hwint07:
	hwint_master	7	; printer
	

;; ----------------------------	
ALIGN	16
hwint08:
	hwint_slave	8	; realtime clock

ALIGN	16
hwint09:
	hwint_slave	9	; irq 2 redirected

ALIGN	16
hwint10:
	hwint_slave	10	; 

ALIGN	16	
hwint11:
	hwint_slave	11

ALIGN	16	
hwint12:
	hwint_slave	12

ALIGN	16
hwint13:
	hwint_slave	13	; FPU exception

ALIGN	16	
hwint14:
	hwint_slave	14	; AT winchester

ALIGN	16	
hwint15:
	hwint_slave	15

	;; 系统调用处理函数
sys_call:	
	call	save
	push	dword [p_proc_ready]
	sti

	push 	edx
	push 	ecx
	push 	ebx
	call	[sys_call_table + eax * 4]
	add	esp, 4*4
	mov	[esi + EAXREG - P_STACKBASE], eax ; 将系统调用返回值放入进程控制块的EAX寄存器，以便在进程恢复以后eax中装的是正确的返回值
	cli
	ret
