obj_dir := ../../build/objs

include ../Makefile.config

target := $(obj_dir)/syscall.o 

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
