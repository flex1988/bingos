CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -std=c99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -I../include -I../include/libc

obj_dir := ../build/objs

target := $(obj_dir)/socket.o $(obj_dir)/net.o
 
all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
