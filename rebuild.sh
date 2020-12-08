#!/bin/sh

set -x

make -C alu clean install || exit

make -C binutils clean install || exit

make -C microcode clean install || exit

make -C sim clean install || exit

mkdir -p lcc/build || exit

make -C lcc clean all || exit

cd lcc && ./install.sh || exit


