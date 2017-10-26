CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

obj_dir := ../build/objs

objs := $(obj_dir)/string.o $(obj_dir)/stdio.o

all: libc

libc: $(objs)
	ar -crv ../build/libc.a $(objs)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -I../include/libc -c -o $@ $< 
