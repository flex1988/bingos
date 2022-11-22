#!/bin/bash

set -e
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

install_binutils()
{
    if [ ! -f binutils-2.30.0.tar.xz ]; then
        wget ftp://sourceware.org/pub/binutils/snapshots/binutils-2.30.0.tar.xz
    fi
    tar -xvf binutils-2.30.0.tar.xz
    rm -rf build-binutils && mkdir build-binutils && cd build-binutils   
    ../binutils-2.30.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
    make && make install
    cd -
    echo "success install binutils"
}

install_gcc()
{
    if [ ! -f gcc-5.4.0.tar.gz ]; then
        wget http://mirrors.concertpass.com/gcc/releases/gcc-5.4.0/gcc-5.4.0.tar.gz
    fi
    tar -xvf gcc-5.4.0.tar.gz
    rm -rf build-gcc && mkdir build-gcc && cd build-gcc && ../gcc-5.4.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
    make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc
    $HOME/opt/cross/bin/$TARGET-gcc --version
    export PATH="$HOME/opt/cross/bin:$PATH"
    cd -
    echo "success install gcc"
}

install_deps()
{
    apt-get install grub2-common grub-pc xorriso
    echo "success install grub"
}

install_binutils
install_gcc
install_deps
