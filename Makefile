arch := x86_64

iso := build/os-$(arch).iso

kernel := build/kernel-$(arch).bin

libc := build/libc.a

lib := build/lib.a

modules := lib	\
	kernel/init \
	kernel/print \
	kernel/mm \
	kernel/graphic \
	kernel/proc	\
	kernel/sys	\
	libc \
	hal \
	fs	\
	drivers \
	userspace	\
	modules		\
	tools

obj_dir:= build/objs

CFLAGS_global := -Wall -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector -D__KERNEL__ 

CC = /root/opt/cross/bin/i686-elf-gcc

NM = /root/opt/cross/bin/i686-elf-nm

AS = /root/opt/cross/bin/i686-elf-as

.PHONY: all $(modules) run iso

linker_script := boot/linker.ld

grub_cfg := boot/grub.cfg

initrd := tools/initrd.img

ext2_module := build/modules/ext2.ko

symbols := build/symbols.o

all: $(modules) $(kernel) $(iso) 

$(modules): Makefile
	test -d build/objs || mkdir -p build/objs
	test -d build/bin || mkdir -p build/bin
	cd $@ && $(MAKE) $(MFLAGS)

symbols: $(obj_files)
	$(CC) -g -O0 -fstack-protector-all -nostdlib -n -T $(linker_script) -o $(kernel) $(wildcard build/objs/*.o) $(libc) $(lib)
	$(NM) build/kernel-x86_64.bin -g|util/generate_symbols.py > kernel/symbols.asm

$(kernel): $(obj_files) $(linker_script)
	cp kernel/symbols.asm build/symbols.asm
	$(AS) --32 build/symbols.asm -o build/symbols.o
	mv build/symbols.o build/objs/symbols.o
	$(CC) -g -O0 -fstack-protector-all -nostdlib -n -T $(linker_script) -o $(kernel) $(wildcard build/objs/*.o) build/objs/symbols.o $(libc) $(lib)

$(iso): $(kernel) $(grub_cfg) $(modules)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	cp $(ext2_module) build/isofiles/boot/ext2.ko
	cp $(initrd) build/isofiles/boot/initrd.img
	grub-mkrescue -o $(iso) build/isofiles 2>/dev/null
	rm -r build/isofiles
	rm $(initrd)

run:
	bochs -q

clean:
	@rm -rf build
