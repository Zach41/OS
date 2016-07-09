	org	0100h		; 为了以后扩展，固定段偏移
	BaseOfStack	equ	0100h


	jmp LABEL_START		;
	%include "fat12hdr.inc"
	%include "pm.inc"
	%include "load.inc"

;; GDT
LABEL_GDT:		Descriptor	0,	0,		0 ; 空描述符
LABEL_DESC_FLAT_C:	Descriptor	0, 	0fffffh,	DA_CR|DA_32|DA_LIMIT_4K ; 0-4G
LABEL_DESC_FLAT_RW:	Descriptor	0, 	0fffffh, 	DA_DRW|DA_32|DA_LIMIT_4K
LABEL_DESC_VIDEO:	Descriptor	0B8000h,0ffffh,		DA_DRW|DA_DPL3

	GdtLen	equ	$-LABEL_GDT
	GdtPtr	dw 	GdtLen-1 			; 段界限
		dd 	BaseOfLoaderPhyAddr + LABEL_GDT ; 基地址

	SelectorFlatC	equ 	LABEL_DESC_FLAT_C  - LABEL_GDT
	SelectorFlatRW	equ	LABEL_DESC_FLAT_RW - LABEL_GDT
	SelectorVideo	equ	LABEL_DESC_VIDEO   - LABEL_GDT
	
	

LABEL_START:
	mov	ax, cs
	mov 	ds, ax
	mov 	es, ax
	mov 	ss, ax
	mov 	sp, BaseOfStack

	mov 	dh, 0	
	call 	DispStr		; "Loading  "

	mov 	word [wSectorNo], SectorNoOfRootDirectory
	xor	ah, ah
	xor 	al, al
	int 	13h		; 软驱复位

LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp 	word [wRootDirSizeForLoop], 0
	jz	LABEL_NO_KERNELBIN
	dec	word [wRootDirSizeForLoop]
	mov	ax, BaseOfKernelFile
	mov 	es, ax
	mov	bx, OffsetOfKernelFile
	mov 	ax, [wSectorNo]
	mov 	cl, 1
	call 	ReadSector

	mov 	si, KernelFileName
	mov	di, OffsetOfKernelFile
	cld
	mov	dx, 10h

LABEL_SEARCH_FOR_KERNELBIN:
	cmp 	dx, 0
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
	dec	dx
	mov	cx, 11

LABEL_CMP_FILENAME:
	cmp 	cx, 0
	jz	LABEL_FILENAME_FOUND
	dec	cx
	lodsb
	cmp	al, byte [es:di]
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT

LABEL_GO_ON:
	inc 	di
	jmp	LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and	di, 0FFE0h	; 本条目开始处
	add	di, 0x20
	mov	si, KernelFileName
	jmp	LABEL_SEARCH_FOR_KERNELBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add 	word [wSectorNo], 1
	jmp 	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_KERNELBIN:
	mov	dh, 2
	call 	DispStr
	jmp	$

LABEL_FILENAME_FOUND:		; 找到了kernel.bin
	mov	ax, RootDirSectors
	and 	di, 0FFE0h

	push 	eax
	mov 	eax, [es:di+01Ch] ; 0x1c 为文件的大小
	mov	dword [dwKernelSize], eax
	pop 	eax

	add	di, 01Ah	; 对应的开始簇号
	mov	cx, word [es:di]
	push 	cx
	add 	cx, ax
	add	cx, DeltaSectorNo
	mov	ax, BaseOfKernelFile
	mov	es, ax
	mov	bx, OffsetOfKernelFile
	mov	ax, cx

LABEL_GOON_LOADING_FILE:
	push 	ax
	push 	bx
	mov	ah, 0Eh
	mov	al, '.'
	mov	bl, 0Fh
	int 	10h
	pop	bx
	pop 	ax

	mov	cl, 1
	call 	ReadSector

	pop	ax
	call	GetFATEntry
	cmp	ax, 0FFFh
	jz	LABEL_FILE_LOADED
	push	ax
	mov	dx, RootDirSectors
	add	ax, dx
	add	ax, DeltaSectorNo
	add	bx, [BPB_BytsPerSec]
	jmp	LABEL_GOON_LOADING_FILE

LABEL_FILE_LOADED:
	call	KillMotor
	mov	dh, 1		
	call	DispStr		; "Ready."

	;; 准备跳入保护模式
	lgdt	[GdtPtr]

	cli

	in 	al, 92h
	or 	al, 00000010b
	out 	92h, al

	mov	eax, cr0
	or 	eax, 1
	mov 	cr0, eax

	;; 进入保护模式
	jmp 	dword SelectorFlatC:(BaseOfLoaderPhyAddr + LABEL_PM_START)

	jmp $
	
	;; Variables
	wRootDirSizeForLoop	dw 	RootDirSectors
	wSectorNo		dw 	0
	bOdd			db 	0
	dwKernelSize 		dd 	0


	KernelFileName		db 	"KERNEL  BIN", 0
	MessageLength		equ	9
	LoadMessage		db 	"Loading  "
	Message1		db 	"Ready.   "
	Message2		db 	"No KERNEL"

;; 显示字符串
DispStr:
	mov	ax, MessageLength
	mul	dh
	add	ax, LoadMessage
	mov	bp, ax
	mov	ax, ds
	mov	es, ax		; es:bp = 字符串
	mov	cx, MessageLength
	mov	ax, 01301h
	mov	bx, 0007h
	mov	dl, 0
	add	dh, 3
	int 	10h
	ret

; 从第ax个扇区开始，将cl个扇区读到es:bx中
ReadSector:
	push bp
	mov bp, sp
	sub esp, 2

	mov byte [bp-2], cl
	push bx
	mov bl, [BPB_SecPerTrk]
	div bl			; al -> y, ah -> z
	inc ah
	mov cl, ah
	mov dh, al
	and dh, 1
	shr al, 1
	mov ch, al
	pop bx

	mov dl, [BS_DrvNum]
.GoOnReading:
	mov ah, 2
	mov al, byte [bp-2]
	int 13h

	jc .GoOnReading		; 若读取错误，CF置１，那就不停地读

	add esp, 2
	pop bp

	ret

GetFATEntry:
	push es
	push bx
	push ax
	mov  ax, BaseOfKernelFile
	sub  ax, 0100h
	mov  es, ax
	pop  ax
	mov  byte [bOdd], 0
	mov  bx, 3
	mul  bx
	mov  bx, 2
	div  bx
	cmp  dx, 0
	jz   LABEL_EVEN
	mov  byte [bOdd], 1
	
LABEL_EVEN:
	xor  dx, dx
	mov  bx, [BPB_BytsPerSec]
	div  bx
	push dx			; ax商，FATEntry所在的扇区相对于FAT起始扇区的扇区号
				; dx余数，FATEntry在扇区内的偏移			
	mov  bx, 0
	add  ax, SectorNoOfFAT1
	mov  cl, 2		; 一次读两个扇区，避免边界错误
	call ReadSector

	pop dx
	add bx, dx
	mov ax, [es:bx]
	cmp byte [bOdd], 1
	jnz LABEL_EVEN_2
	shr ax, 4
	
LABEL_EVEN_2:
	and ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:
	pop bx
	pop es
	ret

;; 关闭驱动马达
KillMotor:
	push 	dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop 	dx
	ret

[SECTION .s32]
	ALIGN	32
[BITS	32]

LABEL_PM_START:
	mov 	ax, SelectorVideo
	mov	gs, ax

	;; mov 	ax, SelectorFlatRW
	;; mov	ds, ax
	;; mov	es, ax
	;; mov 	fs, ax
	;; mov	ss, ax
	;; mov	esp, TopOfStack
	

	mov	ah, 0Fh
	mov	al, 'P'
	mov	[gs:((80*0 + 39) * 2)], ax
	jmp 	$
	
[SECTION .s32]
ALIGN 	32
	
LABEL_DATA:

;; 堆栈在数据段的末尾
StackSpace:	times	1024	db 	0
TopOfStack:	equ	BaseOfLoaderPhyAddr + $
