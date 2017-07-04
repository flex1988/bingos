obj_dir := ../../build/objs

include ../Makefile.inc

target := $(obj_dir)/main.o $(obj_dir)/boot.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include -I../include/libc -c -o $@ $< -ffreestanding -O2 -Wall -Wextra -nostdinc

$(obj_dir)/boot.o:
	nasm -felf32 -o $(obj_dir)/boot.o boot.asm
