obj_dir := ../../build/objs

include ../Makefile.config

target := $(obj_dir)/process.o $(obj_dir)/task.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj_dir)/process.o:
	nasm -felf32 -o $(obj_dir)/process.o process.asm
