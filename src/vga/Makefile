obj_dir := ../../build/objs

target := $(obj_dir)/vga.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include -I../include/libc -c -o $@ $<

