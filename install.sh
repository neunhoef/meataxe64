#!/bin/sh
#
# Script to install meataxe2000 meataxe64 hybrid
#
# Parameter 1: where to install the binaries
#
usage="$0 <dir in which to install (will be deleted)>"
if [ 1 -ne $# ]; then
    echo $usage
    exit 1
fi
dir=`dirname $0`
install=$1
if [ -f $install ]; then
    echo "$0: $1 is a file, terminating"
    exit 1
fi
if [ -d $install ]; then
    rm -rf $install
fi
cd $dir/src
make OS=unix ARCH=em64t rel
mkdir -p $install/m2000
cp -p scr/* derived/unix/em64t/gcc/bin/* $install/m2000
cd ../mtx64
cd ptinstall/jif
mkdir -p $install/ptinstall/jif
for x in *; do sed -e "s?~/install/?$install/m2000/?" < $x > $install/ptinstall/jif/$x; done
cd ../../pinstall
mkdir -p $install/pinstall
for x in *; do sed -e "s?~/ptinstall/?$install/ptinstall/?" < $x > $install/pinstall/$x; done
chmod 755 $install/pinstall/* $install/ptinstall/jif/*
cd ../../git/meataxe64/test
source ./go
makl
compa
cd ..
tar cf - bin | (cd $install/ptinstall; tar xf -)
echo add $install/pinstall/:$install/ptinstall/jif:$install/m2000 to the start of your PATH
