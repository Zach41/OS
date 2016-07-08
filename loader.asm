	org	0100h		; 为了以后扩展，固定段偏移

	mov ax, 0B800h
	mov gs, ax
	mov ah, 0Fh
	mov al, 'L'
	mov [gs:((80*0 + 39) * 2)], ax
	jmp $
	
