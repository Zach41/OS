	%include	"sconst.inc"

	_NR_get_ticks		equ	0
	_NR_write		equ 	1
	_NR_sendrec		equ	2
	_NR_printx		equ	3
	INT_VECTOR_SYS_CALL	equ	0x90

	;; global	get_ticks
	global	write
	global 	sendrec		; IPC系统调用
	global	printx

	bits 32
	[SECTION .text]

get_ticks:
	mov	eax, _NR_get_ticks ; eax内存入系统调用的函数索引
	int 	INT_VECTOR_SYS_CALL ; 调用中断
	ret

write:
	mov 	eax, _NR_write
	mov	ecx, [esp + 4]	; buf
	mov 	edx, [esp + 8]	; len
	int     INT_VECTOR_SYS_CALL
	ret

sendrec:
	mov	eax, _NR_sendrec
	mov	ebx, [esp + 4]	; function
	mov	ecx, [esp + 8]	; src_dest
	mov	edx, [esp + 12]	; p_msg;
	int 	INT_VECTOR_SYS_CALL
	ret

printx:
	mov	eax, _NR_printx
	mov	edx, [esp + 4]
	int 	INT_VECTOR_SYS_CALL
	ret
