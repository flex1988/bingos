CC := /root/opt/cross/bin/i686-elf-gcc

bin_dir := ../build/bin

obj_dir := ../build/libc/objs

libc := ../build/libc.a

target := $(bin_dir)/init $(bin_dir)/sh

all: $(target)

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -I ../include/libc

LDFLAGS := -melf_i386 -T link.ld

$(bin_dir)/init: $(obj_dir)/init.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/init $(obj_dir)/init.o $(libc)

$(bin_dir)/sh: $(obj_dir)/sh.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/sh $(obj_dir)/sh.o $(libc)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -m32 -g -I../include/libc -c -o $@ $<
