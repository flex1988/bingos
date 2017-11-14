obj_dir := ../../build/objs

include ../Makefile.config

target := $(obj_dir)/process.o $(obj_dir)/misc.o $(obj_dir)/sched.o $(obj_dir)/elf.o $(obj_dir)/context.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj_dir)/misc.o:
	nasm -felf32 -o $(obj_dir)/misc.o misc.asm

$(obj_dir)/context.o:
	as --32 -o $(obj_dir)/context.o context.s 
