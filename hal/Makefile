CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -I../include -I../include/libc

obj_dir := ../build/objs

target := $(obj_dir)/hal.o $(obj_dir)/gdt.o $(obj_dir)/cpu.o $(obj_dir)/idt.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
