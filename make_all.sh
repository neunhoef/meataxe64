#!/bin/sh
for x in write.c read.c header.c utils.c ip.c dtou.c; do
  gcc -c $x
done
gcc -o ip read.o header.o utils.o ip.o
gcc -o dtou dtou.o
