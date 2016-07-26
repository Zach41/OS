	extern main2
	extern exit

	bits 	32

	[section .text]

	global 	_start

_start:
	push 	eax
	push	ecx
	call 	main2
	push 	eax
	call	exit

	hlt
