obj_dir := ../../build/objs

include ../Makefile.config

target := $(obj_dir)/process.o $(obj_dir)/misc.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj_dir)/misc.o:
	nasm -felf32 -o $(obj_dir)/misc.o misc.asm
