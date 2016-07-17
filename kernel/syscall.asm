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
