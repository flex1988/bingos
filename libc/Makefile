CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

obj_dir := ../build/libc/objs

objs := $(obj_dir)/string.o $(obj_dir)/stdio.o $(obj_dir)/crt0.o $(obj_dir)/syscall.o $(obj_dir)/vsprintf.o

all: libc

libc: $(objs)
	ar -crv ../build/libc.a $(objs)

$(obj_dir)/%.o: %.c
	test -d ../build/libc/objs || mkdir -p ../build/libc/objs
	$(CC) $(CFLAGS) -I../include/libc -c -o $@ $< 

$(obj_dir)/crt0.o:
	nasm -felf32 -o $(obj_dir)/crt0.o crt0.asm
