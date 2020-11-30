#!/bin/sh
echo Building $1

set -x

../lcc/build/rcc -target=cpu4 $1 > $1.s || exit
../binutils/cpu4as $1.s $1.o || exit
../binutils/cpu4as start.s start.o || exit
../binutils/cpu4ld -o $1.bin start.o $1.o  || exit
hexdump -C $1.bin


