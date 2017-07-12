CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

obj_dir := ../build/objs

target := $(obj_dir)/string.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -I../include/libc -c -o $@ $< 
