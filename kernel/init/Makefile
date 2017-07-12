obj_dir := ../../build/objs

include ../../Makefile.config

target := $(obj_dir)/main.o $(obj_dir)/boot.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 
$(obj_dir)/boot.o:
	nasm -felf32 -o $(obj_dir)/boot.o boot.asm
