CC := /root/opt/cross/bin/i686-elf-gcc

bin_dir := ../build/bin

obj_dir := ../build/libc/objs

libc := ../build/libc.a

lib := ../build/libcommon.a

target := $(bin_dir)/init $(bin_dir)/sh $(bin_dir)/helloworld $(bin_dir)/ls $(bin_dir)/date

.PHONY: $(target)

all: $(target)

CFLAGS := -m32 -std=c99 -nostdlib -nostdinc -fno-builtin -I ../include/libc

LDFLAGS := -melf_i386 -T link.ld

$(bin_dir)/init: $(obj_dir)/init.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/init $(obj_dir)/init.o $(libc) $(lib)

$(bin_dir)/sh: $(obj_dir)/sh.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/sh $(obj_dir)/sh.o $(libc) $(lib)

$(bin_dir)/helloworld: $(obj_dir)/helloworld.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/helloworld $(obj_dir)/helloworld.o $(libc) $(lib)

$(bin_dir)/ls: $(obj_dir)/ls.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/ls $(obj_dir)/ls.o $(libc) $(lib)

$(bin_dir)/date: $(obj_dir)/date.o
	$(LD) $(LDFLAGS) -o $(bin_dir)/date $(obj_dir)/date.o $(libc) $(lib)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -m32 -g -I../include/libc -c -o $@ $<
