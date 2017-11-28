obj_dir := ../build/lib/objs

CC := /root/opt/cross/bin/i686-elf-gcc

CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -I../include -I../include/libc

target := lib

all: $(target)

objs := $(obj_dir)/list.o $(obj_dir)/ordered_array.o

lib: $(objs)
	ar -crv ../build/lib.a $(objs)

$(obj_dir)/%.o: %.c
	test -d $(obj_dir) || mkdir -p $(obj_dir)
	$(CC) $(CFLAGS) -c -o $@ $< 
