arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
uname := $(shell sh -c 'uname -s 2>/dev/null')
target ?= $(arch)-unknown-linux-gnu

ifeq ($(uname),Linux)
	LD=ld
else
	LD=x86_64-elf-ld
endif

CFLAGS = -c -O0 -Wall -Werror -nostdinc -fno-builtin -fno-stack-protector -funsigned-char \
		 -finline-functions -finline-small-functions -findirect-inlining \
		 -finline-functions-called-once -Iinc -m32 -ggdb -gstabs+ -fdump-rtl-expand

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm,\
	build/arch/$(arch)/%.o,$(assembly_source_files))

source_files := $(wildcard src/*.c)
obj_files := $(patsubst src/%.c,build/%.o,$(source_files))

.PHONY: all clean run iso

all: $(kernel) iso

clean:
	@-rm -r build

run: $(iso)
	@qemu-system-x86_64 -cdrom $(iso)

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub2-mkrescue -o $(iso) build/isofiles 2>/dev/null
	@rm -r build/isofiles

obj = src/kernel.o

$(kernel):$(assembly_object_files) $(linker_script) $(obj_files)
	@$(LD) -n --gc-sections -T $(linker_script) -o $(kernel) $(assembly_object_files) $(obj_files)
	
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

build/%.o: src/%.c
	@gcc -c $< -o $@

