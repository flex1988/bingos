### kernel x86

### Build && Run

1. http://wiki.osdev.org/GCC_Cross-Compiler
2. sh build.sh
3. qemu-system-x86_64 -cdrom build/os-x86_64.iso

### GDB Debug

    - gdb build/kernel-x86_64.bin
    - set arch i386:x86-64:intel
    - b kmain
    - target remote :1234
