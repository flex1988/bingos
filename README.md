### kernel x86

`kernel are magic!`

### Build && Run

1. http://wiki.osdev.org/GCC_Cross-Compiler
2. sh build.sh
3. qemu-system-x86_64 -cdrom build/os-x86_64.iso

### GDB Debug

    - gdb build/kernel-x86_64.bin
    - set arch i386:x86-64:intel
    - b kmain
    - target remote :1234

### Memory Layout

```
   / * Memory layout definition
     * +------------+
     * | 0x00100000 | Kernel multiboot header
     * +------------+
     * | 0x00x00000 | some kernel stucts before mmu_init
     * +------------+
     * | 0x20000000 | User mode image loaded address
     * +------------+
     * | 0x30000000 | User stack: 0x30000000~0x30010000
     * +------------+
     * | 0x30010000 | User stack heap bottom
     * +------------+
     * | 0xc0000000 | Kernel heap bottom
     * +------------+
     * | 0xdfffe000 | Init process kernel stack: 0xdfffe000~0xe0000000
     * +------------+
     * | 0xe0000000 | 
     * +------------+
     */
```

### TODO

- [ ] multi core support

- [ ] network tcp/ip stack

- [ ] windows

- [ ] ext2/3 filesystem

- [ ] threads
