BITS 32
extern main
global _start

_start:
	pop eax
	call main
	mov ebx, eax
	mov eax, 0x5
	int 0x80
_wait:
	hlt
	jmp _wait
