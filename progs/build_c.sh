#!/bin/sh
echo Building $1

set -x

../lcc/build/cpp $1 > $1.i || exit
../lcc/build/rcc -target=cpu4 $1.i > $1.s || exit
#../lcc/build/cpp stdlib.c > stdlib.i || exit
#../lcc/build/rcc -target=cpu4 stdlib.c > stdlib.s || exit
../binutils/cpu4as $1.s $1.o || exit
../binutils/cpu4as crt0.s crt0.o || exit
../binutils/cpu4as crt1.s crt1.o || exit
../binutils/cpu4as stdlib.s stdlib.o || exit
../binutils/cpu4ld -o $1.bin crt0.o crt1.o stdlib.o $1.o  || exit

#rm $1.i $1.s $1.o stdlib.i stdlib.o crt0.o crt1.o

#hexdump -C $1.bin

../sim/cpu4sim $1.bin

