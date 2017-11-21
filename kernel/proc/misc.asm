[GLOBAL copy_page_physical]
copy_page_physical:
	push ebx
	pushf

	cli
	
	mov ebx,[esp+12]
	mov ecx,[esp+16]
	
	mov edx,cr0
	and edx,0x7fffffff
	mov cr0,edx
	
	mov edx,1024

.loop
	mov eax,[ebx]
	mov [ecx],eax
	add ebx,4
	add ecx,4
	dec edx
	jnz .loop
	
	mov edx,cr0
	or edx,0x80000000
	mov cr0,edx
	
	popf
	pop ebx
	ret

[GLOBAL read_eip]
read_eip:
	pop eax
	jmp eax

MAGIC equ 0xdecade21

[GLOBAL enter_userspace]
enter_userspace:
	push ebp
	mov ebp, esp
	mov edx,[ebp+12]
	push MAGIC

	mov ax, 0x23

	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax

	mov eax, esp

	push 0x23
	
	push eax
	
	pushf
	pop eax

	or eax, 0x200
	push eax
	push 0x1b

	push long [ebp+8]

	iret
	pop ebp
	ret
