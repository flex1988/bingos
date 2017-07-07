obj_dir := ../../build/objs

include ../Makefile.inc

target := $(obj_dir)/string.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include/libc -c -o $@ $< -nostdinc

