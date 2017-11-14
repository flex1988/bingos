.global do_switch_to_user_mode
.global do_switch_task

do_switch_to_user_mode:
	cli
	mov $0x23, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	mov   %esp, %eax
	pushl $0x23
	pushl %eax
	pushf
	pop   %ebx
	or    $0x200, %ebx
	push  %ebx
	pushl $0x1b
	push  (%eax)
	iret

do_switch_task:
	mov %esp, %eax
	mov 4(%eax), %ecx  # eip
	mov 8(%eax), %esp  # esp
	mov 12(%eax), %ebp  # ebp
	mov 16(%eax), %edx  # page_directory
	mov 20(%eax), %eax  # interrupt_before
	mov %edx, %cr3
	cmp $1, %eax
	je  done
	sti

done:
	jmp *%ecx
