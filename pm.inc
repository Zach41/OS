; Descriptor type
DA_32	     EQU	4000h	; 32位段

DA_DPL0	     EQU 	00h
DA_DPL1	     EQU	20h
DA_DPL2	     EQU	40h
DA_DPL3	     EQU	60h

DA_DR	     EQU	90h	; 存在的只读数据段
DA_DRW	     EQU	92h	; 存在的可读写数据段
DA_DRWA	     EQU	93h	; 存在的已访问的可读写数据段
DA_C	     EQU	98h 	; 存在的只执行代码段属性值
DA_CR	     EQU	9Ah	; 存在的可执行可读代码段
DA_CCO	     EQU	9Ch	; 存在的只执行一致代码段
DA_CCOR	     EQU	9Eh	; 存在的可执行可读一致代码段

; 系统段描述符
DA_LDT       EQU	82h	; LDT
DA_TaskGate  EQU	85h	; Task Gate
DA_386TSS    EQU	89h	; 80386 TSS
DA_386CGate  EQU	8Ch	; 386调用门类型
DA_386IGate  EQU	8Eh	; 386终端门类型
DA_386TGate  EQU	8Fh	; 386陷阱门类型

; --------------------------------------------------------------
; 宏

; 描述符
; param1: segment base address
; param2: segment length
; param3: segment attributes
%macro Descriptor 3
       dw	  %2 & 0FFFFh
       dw 	  %1 & 0FFFFh
       db	 (%1 >> 16) & 0FFh
       dw	 (%3 & 0F0FFh) | ((%2 >> 8) & 0F00h)
       db 	 (%1 >> 24) & 0FFh
%endmacro

