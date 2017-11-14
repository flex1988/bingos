BITS 32
global _start
_start:
	pop eax
	extern main
	call main

	; 0x0 is syscall_exit
	mov ebx, eax
	mov eax, 0x0
	int 0x80
