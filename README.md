## BingOS

### Build Cross Compiler

- http://wiki.osdev.org/GCC_Cross-Compiler

#### Build binutils

```
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
```

1. wget ftp://sourceware.org/pub/binutils/snapshots/binutils-2.30.0.tar.xz
2. tar -xvf binutils-2.30.0.tar.xz
3. mkdir build-binutils & cd build-binutils
4. ../binutils-2.30.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
5. make & make install

#### Build GCC

```
1. GMP (libgmp3-dev on apt-based systems, dev-libs/gmp on Gentoo, gmp-devel on Fedora, libgmp-devel on Cygwin)
2. MPFR (libmpfr-dev on apt-based systems, dev-libs/mpfr on Gentoo, mpfr-devel on Fedora, libmpfr-devel on Cygwin)
3. Mpc (libmpc-dev on apt-based systems, dev-libs/mpc on Gentoo, libmpc-devel on Fedora, libmpc-devel on Cygwin)
```

1. wget http://mirrors-usa.go-parts.com/gcc/releases/gcc-5.4.0/gcc-5.4.0.tar.gz
2. tar -xvf gcc-5.4.0.tar.gz & cd gcc-5.4.0
3. mkdir build-gcc & cd build-gcc & ../gcc-5.4.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
4. make all-gcc & make all-target-libgcc & make install-gcc & make install-target-libgcc
5. $HOME/opt/cross/bin/$TARGET-gcc --version
6. export PATH="$HOME/opt/cross/bin:$PATH"


### Build && Run

1. sh build.sh
2. qemu-system-x86_64 -cdrom build/os-x86_64.iso

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
