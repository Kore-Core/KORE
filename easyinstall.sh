#!/bin/bash
echo -n "This is a very basic script for quick install of the KORE wallet. It does not yet check to see if you have enough memory for compiling or that you have required dependencies. If you understand this and wish to continue please enter y, if not enter n: "
read answer
if [ "$answer" != "${answer#[Yy]}" ] ;then
    echo
    echo starting compile process
    sleep 4
    ./autogen.sh
    ./configure
    sudo make
    sudo make install
else
    echo
    echo you have selected no, please verify you have the depends/memory required or install the wallet manually.
fi
