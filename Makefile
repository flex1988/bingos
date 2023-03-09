arch := x86_64

iso := build/os-$(arch).iso

kernel := build/kernel-$(arch).bin

libc := build/libc.a

lib := build/lib.a

grub := grub2

modules := lib	\
	kernel	\
	libc \
	hal \
	fs	\
	net	\
	drivers \
	userspace	\
	modules		\
	tools

obj_dir:= build/objs

CFLAGS_global := -Wall -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector -D__KERNEL__ 

CC = /root/opt/cross/bin/i686-elf-gcc

NM = /root/opt/cross/bin/i686-elf-nm

AS = /root/opt/cross/bin/i686-elf-as

.PHONY: all $(modules) run iso $(kernel)

linker_script := boot/linker.ld

grub_cfg := boot/grub.cfg

initrd := tools/initrd.img

symbols := build/symbols.o

all: $(modules) $(kernel) $(iso) 

$(modules): Makefile
	test -d build/objs || mkdir -p build/objs
	test -d build/bin || mkdir -p build/bin
	cd $@ && $(MAKE) $(MFLAGS)

symbols: $(obj_files)
	test -d build/objs || mkdir -p build/objs
	$(CC) -g -O0 -fstack-protector-all -nostdlib -n -T $(linker_script) -o $(kernel) $(wildcard build/objs/*.o) $(libc) $(lib)
	$(NM) build/kernel-x86_64.bin -g|tools/generate_symbols.py > kernel/symbols.asm
	$(AS) --32 kernel/symbols.asm -o build/objs/symbols.o

$(kernel): $(obj_files) $(linker_script)
	$(CC) -g -O0 -fstack-protector-all -nostdlib -n -T $(linker_script) -o $(kernel) $(wildcard build/objs/*.o)  $(libc) $(lib)

$(iso): $(kernel) $(grub_cfg) $(modules)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	cp build/modules/* build/isofiles/boot
	cp $(initrd) build/isofiles/boot/initrd.img
	$(grub)-mkrescue -o $(iso) build/isofiles
	rm -r build/isofiles
	rm $(initrd)

run:
	bochs -q

clean:
	@rm -rf build
