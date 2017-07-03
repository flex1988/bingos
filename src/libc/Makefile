obj_dir := ../../build/objs

include ../Makefile.inc

target := $(obj_dir)/string.o

CFLAGS := -Wall -nostdlib -nostdinc -fno-stack-protector -D__KERNEL__

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include/libc -c -o $@ $< 

