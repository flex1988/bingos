obj_dir := ../build/objs

include Makefile.config

target := $(obj_dir)/process.o $(obj_dir)/misc.o $(obj_dir)/sched.o $(obj_dir)/elf.o $(obj_dir)/syscall.o $(obj_dir)/module.o $(obj_dir)/timer.o $(obj_dir)/printk.o $(obj_dir)/vsprintf.o $(obj_dir)/mmu.o $(obj_dir)/frame.o $(obj_dir)/mmap.o $(obj_dir)/kmalloc.o $(obj_dir)/main.o $(obj_dir)/boot.o $(obj_dir)/console.o $(obj_dir)/vga.o $(obj_dir)/vesa.o $(obj_dir)/drawing.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj_dir)/misc.o:
	nasm -felf32 -o $(obj_dir)/misc.o misc.asm

$(obj_dir)/boot.o:
	nasm -g -O0 -felf32 -o $(obj_dir)/boot.o boot.asm
