	%include "sconst.inc"

	extern 	gdt_ptr
	extern  idt_ptr
	extern 	disp_pos
	extern  p_proc_ready
	extern  tss
	

	extern 	cstart
	extern	exception_handler
	extern	kernel_main
	extern	spurious_irq	; 外部中断处理函数

[SECTION .bss]
	StackSpace	resb	2*1024
	StackTop:

[SECTION .text]
	global 	_start

	global	restart		; 进程运行入口

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
	push	%1
	call 	spurious_irq
	add	esp, 4
	hlt
%endmacro

%macro	hwint_slave 	1
	push 	%1
	call	spurious_irq
	add	esp, 4
	hlt
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

	pop 	gs
	pop 	fs
	pop 	es
	pop 	ds
	popad

	add 	esp, 4		; 略过retaddr
	iretd
	

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
	iretd	 	; the clock

ALIGN	16
hwint01:
	hwint_master	1	; keyboard

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
