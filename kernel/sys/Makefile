obj_dir := ../../build/objs

include ../Makefile.config

target := $(obj_dir)/syscall.o $(obj_dir)/module.o $(obj_dir)/timer.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
