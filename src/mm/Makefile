obj_dir := ../../build/objs

include ../Makefile.inc

target := $(obj_dir)/page.o

CFLAGS := -Wall -nostdlib -nostdinc -fno-builtin

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include -I../include/libc -fno-builtin -c -o $@ $< 
