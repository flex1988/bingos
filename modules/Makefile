CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -std=c99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

LDFLAGS := -melf_i386 -T link.ld

obj_dir := ../build/modules

target := $(obj_dir)/ext2.ko $(obj_dir)/ata.ko

all: $(target)

$(obj_dir)/%.ko: %.c
	test -d $(obj_dir) || mkdir -p $(obj_dir)
	$(CC) $(CFLAGS) -T link.ld -I../include -I../include/libc -c -o $@ $< 
