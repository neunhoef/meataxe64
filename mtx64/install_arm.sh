#!/bin/bash
#
# Script to install ARM versions of meataxe2000 meataxe64 hybrid
#
# Parameter 1: where to install the binaries
#
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
cd $dir/../m2000
make OS=unix ARCH=arm rel
mkdir -p $install/m2000
cp -p scr/* derived/unix/arm/gcc/bin/* $install/m2000
cd ../mtx64
cd ptinstall/jif
mkdir -p $install/ptinstall/jif
for x in *; do sed -e "s?~/install/?$install/m2000/?" < $x > $install/ptinstall/jif/$x; done
cd ../../pinstall
mkdir -p $install/pinstall
for x in *; do sed -e "s?~/ptinstall/?$install/ptinstall/?" < $x > $install/pinstall/$x; done
chmod 755 $install/pinstall/* $install/ptinstall/jif/*
cd ../../m64
mkdir $install/ptinstall/bin
make OS=unix ARCH=arm rel
cp -p derived/unix/arm/gcc/bin/* $install/ptinstall/bin
echo add ${install}/pinstall:${install}/ptinstall/jif:${install}/ptinstall/bin:${install}/m2000 to the start of your PATH
echo If you only want mtx2000 then omit pinstall and ptinstall
echo If you only want mtx64 then just use ${install}/ptinstall/bin
