arch := x86_64

iso := build/os-$(arch).iso

kernel := build/kernel-$(arch).bin

modules := src/init src/vga src/print src/mm src/libc

obj_dir:= build/objs

CFLAGS_global := -Wall -nostdlib -nostdinc -fno-builtin -fno-stack-protector -D__KERNEL__ 

CC = /root/opt/cross/bin/i686-elf-gcc

.PHONY: all clean run iso

linker_script := boot/linker.ld

grub_cfg := boot/grub.cfg

assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm,\
	build/arch/$(arch)/%.o,$(assembly_source_files))

all: $(modules) $(kernel) $(iso)

$(modules): Makefile
	test -d build/objs || mkdir -p build/objs
	cd $@ && $(MAKE) $(MFLAGS)

$(kernel): $(obj_files) $(linker_script) 
	$(CC) -nostdlib -n -T $(linker_script) -o $(kernel) $(wildcard build/objs/*.o)

$(iso): $(kernel) $(grub_cfg) $(modules)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2>/dev/null
	@rm -r build/isofiles

clean:
	@rm -rf build
