#!/usr/bin/env bash

wget http://ftp.gnu.org/gnu/gdb/gdb-8.2.1.tar.xz
tar -xf gdb-8.2.1.tar.xz
cd gdb-8.2.1/
./configure
make
sudo cp gdb/gdb /usr/local/bin/gdb