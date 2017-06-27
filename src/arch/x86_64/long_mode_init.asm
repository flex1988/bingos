global long_mode_start

section .text
bits 64
long_mode_start:
	mov ax,0
	mov ss,ax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	extern kmain
	call kmain

	mov rax,0x4f724f204f534f4f
	mov [0xb8000], rax
    mov rax, 0x4f724f754f744f65
    mov [0xb8008], rax
    mov rax, 0x4f214f644f654f6e
    mov [0xb8010], rax
    hlt
