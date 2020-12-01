#!/bin/sh
echo Building $1

set -x

../binutils/cpu4as $1 $1.o || exit
../binutils/cpu4ld -o $1.bin $1.o  || exit
hexdump -C $1.bin
../sim/cpu4sim $1.bin

