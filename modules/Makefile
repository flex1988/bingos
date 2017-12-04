CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -std=c99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

obj_dir := ../build/modules/objs

target := $(obj_dir)/ext2.o

all: $(target)

$(obj_dir)/%.o: %.c
	test -d $(obj_dir) || mkdir -p $(obj_dir)
	$(CC) $(CFLAGS) -I../include -I../include/libc -c -o $@ $< 
