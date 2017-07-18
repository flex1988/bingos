obj_dir := ../build/objs

CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -I../include -I../include/libc

target := $(obj_dir)/ordered_array.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 
