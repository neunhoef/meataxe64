#!/bin/bash
#
# A test for zec znd zrec
#
# We check that applying the row and column selects, and then
# mulitplying on the left by
# M 0
# K 1
#
# Where M is the multiplier and K the cleaner
# produces
# -1 R
#  0 0
# where R is the remnant
#
# Inputs:-
# 1: A matrix to be echelised
# 2: rs
# 3: cs
# 4: multiplier
# 5: cleaner
# 6: remnant
#
# Return codes:-
# 0: Success
# non-zero an exit code, plus a message
#
# Algorithm
# We can do this in parts, thus reducing the overall work
# Do a row extract on A to get S and N, then K*S+N must be zero
# Now column extract S to get SS and SN
# Then M*SS must be -1, and M*SN must be R
#
usage="ectest.sh <input> <row select> <column select> <multiplier> <cleaner> <remnant>"
if [ 6 != $# ]; then
  echo $usage 1>&2
  exit 0
fi
in=$1
rs=$2
cs=$3
m=$4
k=$5
r=$6
tmp=tmp${PPID}
char=`zchar ${in}`
prime=`zprime ${in}`
nor=`znor ${in}`
noc=`znoc ${in}`
nor2=`znor ${k}`
nor3=`znor ${m}`
# Row select the input
zrx ${rs} ${in} ${tmp}.{1,2}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to row extract ${in} using ${rs}, terminating" 2>&1
  exit $ret
fi
zmu ${k} ${tmp}.1 ${tmp}.3
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to multiply ${k} and ${tmp}.1, terminating" 2>&1
  exit $ret
fi
zad ${tmp}.{2,3,4}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to add ${tmp}.2 and ${tmp}.3, terminating" 2>&1
  exit $ret
fi
# Produce 0 matrix of correct size and check against ${tmp}.4
zsid ${tmp}.5 ${prime} ${nor2} ${noc} 0
zdiff ${tmp}.{4,5}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Results differ, failed" 1>&2
  exit $ret
else
  echo "Cleaning worked"
fi
# Now the multiplier part
zmu ${m} ${tmp}.{1,6}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to multiply ${m} and ${tmp}.1, terminating" 2>&1
  exit $ret
fi
zcx ${cs} ${tmp}.{6,7,8}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to column select ${tmp}.6 with ${cs}, terminating" 2>&1
  exit $ret
fi
# 7 should be -1, 8 should be R
zdiff ${tmp}.8 ${r}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Results differ, failed" 1>&2
  exit $ret
else
  echo "Remnant worked"
fi
let minus1=${char}-1
zsid ${tmp}.9 ${prime} ${nor3} ${nor3} ${minus1}
zdiff ${tmp}.{7,9}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Results differ, failed" 1>&2
  exit $ret
else
  echo "-1 worked"
fi
rm -f ${tmp}.{1,2,3,4,5,6,7,8,9}
