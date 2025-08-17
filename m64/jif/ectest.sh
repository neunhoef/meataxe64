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
# were R is the remnant
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
# Row select the input and glue back together using splice
# Column select the result, transpose both parts, spliace then transpose again
# We now have something we can multiply on the left
# Combine M and K to produce the overall multiplier
# Multiply giving T
# Produce -1, transpose R, splice, transpose again
# Produce 0 matrix of correct size and splice on giving U
# Check U == T
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
# Row select the input and glue back together using splice
zrx ${rs} ${in} ${tmp}.{1,2}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to row extract ${in} using ${rs}, terminating\n" 2>&1
  exit $ret
fi
ret=$?
zcn ${tmp}.{1,2,3}
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.1 and ${tmp}.2, terminating\n" 2>&1
  exit $ret
fi
# Column select the result, transpose both parts, splice then transpose again
ztr ${tmp}.{3,4}
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${tmp}.3, terminating\n" 2>&1
  exit $ret
fi
zrx ${cs} ${tmp}.{4,5,6}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to row extract ${tmp}.3 using ${cs}, terminating\n" 2>&1
  exit $ret
fi
zcn ${tmp}.{5,6,7}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.4 and ${tmp}.5, terminating\n" 2>&1
  exit $ret
fi
ztr ${tmp}.{7,8}
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${tmp}.7, terminating\n" 2>&1
  exit $ret
fi
# We now have something we can multiply on the left
# Combine M and K to produce the overall multiplier
char=`zchar ${in}`
prime=`zprime ${in}`
nor=`znor ${in}`
noc=`znoc ${in}`
nor1=`znor ${m}`
noc1=`znoc ${m}`
# We need a 0 matrix with nor1 rows and nor-nor1 columns
# except we want it transposed for the following step
let noc2=${nor}-${nor1}
zsid ${tmp}.9 ${prime} ${noc2} ${nor1} 0
# This and $m form the top half, so column riffle with trivial string
# which we do by transpose, cn, transpose
ztr $m ${tmp}.10
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${m}, terminating\n" 2>&1
  exit $ret
fi
zcn ${tmp}.{10,9,11}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.10 and ${tmp}.8, terminating\n" 2>&1
  exit $ret
fi
ztr ${tmp}.{11,12}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${tmp}.11, terminating\n" 2>&1
  exit $ret
fi
# Now we need K and and identity to glue onto the bottom
zid ${tmp}.13 ${prime} ${noc2} ${noc2}
ztr ${k} ${tmp}.14
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${k}, terminating\n" 2>&1
  exit $ret
fi
zcn ${tmp}.{14,13,15}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.14 and ${tmp}.13, terminating\n" 2>&1
  exit $ret
fi
ztr ${tmp}.{15,16}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${tmp}.15, terminating\n" 2>&1
  exit $ret
fi
# Now glue the two together
zcn ${tmp}.{12,16,17}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.14 and ${tmp}.13, terminating\n" 2>&1
  exit $ret
fi
# Multiply giving T
zmu ${tmp}.{17,8,18}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to multiply ${tmp}.17 and ${tmp}.6, terminating\n" 2>&1
  exit $ret
fi
# Produce -1, transpose R, splice, transpose again
let minus1=${char}-1
zsid ${tmp}.19 ${prime} ${nor1} ${nor1} ${minus1}
ztr ${r} ${tmp}.20
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${tmp}.20, terminating\n" 2>&1
  exit $ret
fi
zcn ${tmp}.{19,20,21}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.19 and ${tmp}.20, terminating\n" 2>&1
  exit $ret
fi
ztr ${tmp}.{21,22}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to transpose ${tmp}.21, terminating\n" 2>&1
  exit $ret
fi
# Produce 0 matrix of correct size and splice on giving U
zsid ${tmp}.23 ${prime} ${noc2} ${noc} 0
zcn ${tmp}.{22,23,24}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to combine ${tmp}.22 and ${tmp}.23, terminating\n" 2>&1
  exit $ret
fi
# Check U == T
zdiff ${tmp}.{18,24}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Results differ, failed\n" 1>&2
  exit $ret
fi
rm ${tmp}.{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24}
