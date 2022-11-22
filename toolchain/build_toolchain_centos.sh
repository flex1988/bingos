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
    echo "successfully install binutils"
}

install_gmp()
{
    if [ ! -f gmp-6.1.0.tar.bz2 ]; then
        wget ftp://gcc.gnu.org/pub/gcc/infrastructure/gmp-6.1.0.tar.bz2
    fi
    tar xvf gmp-6.1.0.tar.bz2
    rm -rf gmp && mkdir gmp && cd gmp
    ../gmp-6.1.0/configure && make && make install
    cd -
    echo "successfully install gmp"
}

install_mpfr()
{
    if [ ! -f mpfr-3.1.4.tar.bz2 ]; then
        wget ftp://gcc.gnu.org/pub/gcc/infrastructure/mpfr-3.1.4.tar.bz2
    fi
    tar xvf mpfr-3.1.4.tar.bz2
    rm -rf mpfr && mkdir mpfr && cd mpfr
    ../mpfr-3.1.4/configure && make && make install
    cd -
    echo "successfully install mpfr"
}

install_mpc()
{
    if [ ! -f mpc-1.0.3.tar.gz ]; then
        wget ftp://gcc.gnu.org/pub/gcc/infrastructure/mpc-1.0.3.tar.gz
    fi
    tar xvf mpc-1.0.3.tar.gz
    rm -rf mpc && mkdir mpc && cd mpc
    ../mpc-1.0.3/configure && make && make install
    cd -
    echo "successfully install mpc"
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
    echo "successfuly install gcc"
}

install_nasm()
{
    yum install -y nasm
}

install_xorriso()
{
    yum install -y xorriso
}


install_binutils
install_gmp
install_mpfr
install_mpc
install_gcc
install_nasm
install_xorriso
