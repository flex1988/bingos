obj_dir := ../../build/objs

include ../Makefile.config

target := $(obj_dir)/console.o $(obj_dir)/vga.o $(obj_dir)/vesa.o $(obj_dir)/drawing.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
