obj_dir := ../../build/objs

target := $(obj_dir)/printk.o $(obj_dir)/vsprintf.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include/libc -c -o $@ $< 

