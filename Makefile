arch := x86_64

iso := build/os-$(arch).iso

kernel := build/kernel-$(arch).bin

modules := kernel/init \
	kernel/print \
	kernel/mm \
	kernel/vga \
	kernel/proc	\
	kernel/sys	\
	libc \
	hal \
	lib	\
	fs	\
	drivers \
	userspace	\
	tools

obj_dir:= build/objs

CFLAGS_global := -Wall -nostdlib -nostdinc -fno-builtin -fno-stack-protector -D__KERNEL__ 

CC = /root/opt/cross/bin/i686-elf-gcc

.PHONY: all $(modules) run iso

linker_script := boot/linker.ld

grub_cfg := boot/grub.cfg

initrd := tools/initrd.img

all: $(modules) $(kernel) $(iso)

$(modules): Makefile
	test -d build/objs || mkdir -p build/objs
	test -d build/bin || mkdir -p build/bin
	cd $@ && $(MAKE) $(MFLAGS)

$(kernel): $(obj_files) $(linker_script) 
	$(CC) -nostdlib -n -T $(linker_script) -o $(kernel) $(wildcard build/objs/*.o)

$(iso): $(kernel) $(grub_cfg) $(modules)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	mv $(initrd) build/isofiles/boot/initrd.img
	grub-mkrescue -o $(iso) build/isofiles 2>/dev/null
	rm -r build/isofiles

clean:
	@rm -rf build
