CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -I../include -I../include/libc

obj_dir := ../build/objs

target := $(obj_dir)/isr.o $(obj_dir)/descriptor.o $(obj_dir)/interrupt.o $(obj_dir)/gdt.o $(obj_dir)/timer.o $(obj_dir)/common.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj_dir)/interrupt.o:
	nasm -felf32 -o $(obj_dir)/interrupt.o interrupt.asm

$(obj_dir)/gdt.o:
	nasm -felf32 -o $(obj_dir)/gdt.o gdt.asm
