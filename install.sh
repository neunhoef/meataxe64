#!/bin/sh
#
# Script to install meataxe2000 meataxe64 hybrid
#
# Parameter 1: where to install the binaries
#
usage="$0 <dir in which to install (will be deleted)>
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
cd $dir/.. # Get into the root of the installed source
cd src
make OS=unix ARCH=em64t rel
mkdir -p $install/m2000
cp -p scr/* derived/unix/em64t/gcc/bin/* $install/m2000
cd ../mtx64
tar cf - . | (cd $install | tar xf -)
cd ../git/meataxe64/test
source go
makl
compa
cd ..
tar cf - bin | (cd $instal/ptinstall; tar xf -)
echo "set up PATH by PATH="$install/pinstall/:$install/ptinstall/jif:$install/m2000:$PATH"
