CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -std=c99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -I../include -I../include/libc

obj_dir := ../build/objs

target := $(obj_dir)/fs.o $(obj_dir)/initrd.o $(obj_dir)/open.o $(obj_dir)/pipe.o $(obj_dir)/read_write.o $(obj_dir)/exec.o $(obj_dir)/namei.o $(obj_dir)/stat.o $(obj_dir)/devfs.o
 
all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
