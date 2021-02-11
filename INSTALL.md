# Getting libaec

The source code of libaec is hosted at DKRZ GitLab.

## Source code and binary releases

The latest releases of libaec can be downloaded at the following
location:

  https://gitlab.dkrz.de/k202009/libaec/tags

## Developer snapshot

  git clone https://gitlab.dkrz.de/k202009/libaec.git


# Installation

## General considerations

Libaec achieves the best performance on 64 bit systems. The library
will work correctly on 32 bit systems but encoding and decoding
performance will be much lower.

## Installation from source code release with configure

The most common installation procedure on Unix-like systems looks as
follows:

Unpack the tar archive and change into the unpacked directory.

  mkdir build
  cd build
  ../configure
  make check install

## Installation from source code release with CMake

As an alternative, you can use CMake to install libaec.

Unpack the tar archive and change into the unpacked directory.

  mkdir build
  cd build
  cmake ..
  make install

You can set options for compiling using the CMake GUI by replacing the cmake
command with

  cmake-gui ..

or by setting the options manually, e.g.

  cmake -DCMAKE_INSTALL_PREFIX=~/local ..

in order to set the install prefix to ~/local

CMake can also generate project files for Microsoft Visual Studio when
used in Windows.

## Installation from cloned repository

The configure script is not included in the repository. You can
generate it with autotools and gnulib:

  cd libaec
  gnulib-tool --import lib-symbol-visibility
  autoreconf -iv
  mkdir build
  cd build
  ../configure
  make check install


# Intel compiler settings

The Intel compiler can improve performance by vectorizing certain
parts of the code on x86 architectures. Assuming your CPU supports
AVX2, the following options will increase encoding speed.

  ../configure CC=icc
  make CFLAGS="-O3 -xCORE-AVX2" bench

On a 3.4 GHz E3-1240 v3 we see more than 400 MiB/s for encoding
typical data.

Using other SIMD instruction sets on older CPUs may also help.
