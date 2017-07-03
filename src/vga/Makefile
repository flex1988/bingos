obj_dir := ../../build/objs

include ../Makefile.inc

target := $(obj_dir)/vga.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include -I../include/libc -c -o $@ $< -std=gnu99 -ffreestanding -O2 -Wall -Wextra
