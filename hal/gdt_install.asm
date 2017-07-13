section .text
global gdt_install:function (gdt_install.end - gdt_install)
gdt_install:
	mov eax, [esp+4]
	lgdt [eax]
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax
	jmp 0x80:.install
.install:
	ret
.end:
