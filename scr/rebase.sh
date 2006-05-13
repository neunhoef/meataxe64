#!/bin/sh
#
# $Id: rebase.sh,v 1.2 2006/05/13 15:43:01 jon Exp $
#
# Script to rebase a set of representations according to new scripts
# This is for use in tensor condensation, where the delta words used
# to construct the symmetry basis must be the same as the scripts used
# to create the irreducibles from which the Q and P matrices are made
#
# Inputs
# $1: file of delta words
# $2: file of original representations
# $3: number of generators
#
# Return codes
# 1: some sort of error
# 0: ok
#
name=$0
usage="$name: usage: $name <deltas> <irreducibles> <count of generators>"
if [ $# -ne 3 ]; then
  echo "$usage"
  exit 1;
fi
. functions
deltas=$1
irrs=$2
n=$3
if [ ! -e $deltas -o ! -e $irrs ]; then
  echo "$0: one of $deltas or $irrs not found, terminating"
  exit 1
fi
i=0
for x in `cat $1`; do
  d[$i]=$x
  let i=$i+1
done
i=0
for y in `cat $2`; do
  gens=
  z=1
  while [ $z -le $n ]; do
    gens="$gens ${y}_$z"
    let z=$z+1
  done
  new_irred_with_script 10000 ${d[$i]} $gens
  let i=$i+1
done
