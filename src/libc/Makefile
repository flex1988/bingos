obj_dir := ../../build/objs

target := $(obj_dir)/string.o

CFLAGS_global := -Wall -nostdlib -nostdinc -fno-builtin -fno-stack-protector -D__KERNEL__ -D_X86_

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include/libc -c -o $@ $< 

