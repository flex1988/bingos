CC := /root/opt/cross/bin/i686-elf-gcc

AS := nasm

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

obj_dir := ../build/libc/objs

objs := $(obj_dir)/string.o $(obj_dir)/stdio.o $(obj_dir)/crt0.o $(obj_dir)/syscall.o $(obj_dir)/vsprintf.o $(obj_dir)/errno.o $(obj_dir)/stdlib.o $(obj_dir)/malloc.o $(obj_dir)/dirent.o $(obj_dir)/socket.o $(obj_dir)/time.o $(obj_dir)/strftime.o

all: libc

libc: $(objs)
	ar -crv ../build/libc.a $(objs) ../build/libcommon.a

$(obj_dir)/%.o: %.c
	test -d ../build/libc/objs || mkdir -p ../build/libc/objs
	$(CC) $(CFLAGS) -I../include/libc -I../include -c -o $@ $< 

$(obj_dir)/crt0.o:
	$(AS) -felf32 -o $(obj_dir)/crt0.o crt0.asm
