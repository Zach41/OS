org 	07c00h
BaseOfStack 	equ	07c00h

BaseOfLoader		equ	09000h	; Loader.bin被加载到的段地址
OffsetOfLoader		equ	0100h	; Loader.bin被加载到的偏移地址
RootDirSectors		equ	14	; 根目录占用空间
SectorNoOfFAT1		equ	1	; FAT1 的第一个扇区号
DeltaSectorNo		equ	17
	
SectorNoOfRootDirectory	equ	19	; Root Dir的第一个扇区号
	
	jmp short LABEL_START
	nop			; FAT引导扇区格式规定

	;; FAT12 Headers, 含义参照P104表格
	BS_OEMName	DB	'ForrestY' 
	BPB_BytsPerSec	Dw	512	   
	BPB_SecPerClus	DB	1	   
	BPB_RsvdSecCnt	DW	1	   
	BPB_NumFATs	DB	2	   
	BPB_RootEntCnt	DW	224	   
	BPB_TotSec16	DW	2880
	BPB_Media	DB	0xF0
	BPB_FATSz16	DW	9
	BPB_SecPerTrk	DW	18
	BPB_NumHeads 	DW	2
	BPB_HiddSec	DD	0
	BPB_TotSec32 	DD	0
	BS_DrvNum	DB	0
	BS_Reserved1	DB	0
	BS_BootSig	DB	29h
	BS_VolID	DD	0
	BS_VolLab	DB	'OrangeS0.02'
	BS_FileSysType	DB	'FAT12   '

LABEL_START:
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, BaseOfStack

	;; 清屏
	mov ax, 0600h
	mov bx, 0700h
	mov cx, 0
	mov dx, 0184fh
	int 10h

	mov dh, 0
	call DispStr

	xor ah, ah
	xor dl, dl
	int 13h			;软驱复位

	;; 在Ａ盘的根目录寻找loader.bin
	mov word [wSectorNo], SectorNoOfRootDirectory
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp word [wRootDirSizeForLoop], 0
	jz  LABEL_NO_LOADERBIN
	dec word [wRootDirSizeForLoop]
	mov ax, BaseOfLoader
	mov es, ax
	mov bx, OffsetOfLoader
	mov ax, [wSectorNo]
	mov cl, 1
	call ReadSector

	mov si, LoaderFileName
	mov di, OffsetOfLoader
	cld
	mov dx, 10h		; 一个扇区共有32个根目录条目
LABEL_SEARCH_FOR_LOADERBIN:
	cmp dx, 0
	jz  LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
	dec dx
	mov cx, 11
LABEL_CMP_FILENAME:
	cmp cx, 0
	jz  LABEL_FILENAME_FOUND
	dec cx
	lodsb
	cmp al, [es:di]
	jz  LABEL_GO_ON
	jmp LABEL_DIFFERENT

LABEL_GO_ON:
	inc di
	jmp LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and di, 0FFE0h		; 指向本条目开头
	add di, 20h
	mov si, LoaderFileName
	jmp LABEL_SEARCH_FOR_LOADERBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add word [wSectorNo], 1
	jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov dh, 2
	call DispStr

LABEL_FILENAME_FOUND:
	mov ax, RootDirSectors
	and di, 0FFE0h		; 当前条目的开始
	add di, 01Ah		; 该条目中表示第一个sector的位置
	mov cx, word [es:di]
	push cx
	add cx, ax
	add cx, DeltaSectorNo
	mov ax, BaseOfLoader
	mov es, ax
	mov bx, OffsetOfLoader
	mov ax, cx

LABEL_GOON_LOADING_FILE:
	push ax
	push bx
	mov  ah, 0Eh
	mov  al, '.'
	int  10h
	pop  bx
	pop  ax

	mov  cl, 1
	call ReadSector
	pop  ax
	call GetFATEntry
	cmp  ax, 0FFFh
	jz   LABEL_FILE_LOADED
	push ax
	mov  dx, RootDirSectors
	add  ax, dx
	add  ax, DeltaSectorNo
	add  bx, [BPB_BytsPerSec]
	jmp  LABEL_GOON_LOADING_FILE

LABEL_FILE_LOADED:
	mov  dh, 1
	call DispStr		; "Ready."

	jmp  BaseOfLoader:OffsetOfLoader ; 开始执行loader.bin的代码


;; 变量
	wRootDirSizeForLoop	dw 	RootDirSectors
	wSectorNo		dw 	0
	bOdd 			db 	0

	;; 字符串
	LoaderFileName		db 	"LOADER  BIN", 0
	MessageLength		equ	9
	BootMessage		db 	"Booting  "
	Message1		db    	"Ready.   "
	Message2		db 	"No Loader"

DispStr:
	mov ax, MessageLength
	mul dh
	add ax, BootMessage
	mov bp, ax
	mov ax, ds
	mov es, ax		; ES:BP = 串地址
	mov cx, MessageLength
	mov ax, 01301h
	mov bx, 0007h
	mov dl, 0
	int 10h
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
	mov  ax, BaseOfLoader
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
	
times 510-($-$$)	db	0
dw	0xaa55			; 结束标志
	
