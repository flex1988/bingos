BITS 32
global _start
_start:
	pop eax
	extern main
	call main
	
	mov ebx, eax
	mov eax, 0x5
	int 0x80
