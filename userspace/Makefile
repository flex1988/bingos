CC := /root/opt/cross/bin/i686-elf-gcc

bin_dir := ../build/bin

obj_dir := ../build/objs

libc := ../build/libc.a

target := $(bin_dir)/init

all: $(target)

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -I ../include/libc

$(bin_dir)/%: %.c
	$(CC) $(CFLAGS) $< $(libc) -o $@
