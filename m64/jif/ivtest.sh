#!/bin/bash
#
# A test for ziv1
#
# We check that the result is the required inverse
#
# Inputs:-
# 1: A matrix which has been inverted
# 2: the inverse
usage="ivtest.sh <input> <inverse>"
if [ 2 != $# ]; then
  echo $usage 1>&2
  exit 0
fi
in=$1
iv=$2
tmp=tmp${PPID}
char=`zchar ${in}`
prime=`zprime ${in}`
nor=`znor ${in}`
noc=`znoc ${in}`
nor2=`znor ${iv}`
if [ ${nor} -ne ${noc} ]; then
    echo "$in is not square, can't be inverted" 1>&2
    exit 1
fi
if [ ${nor} -ne ${nor2} ]; then
    echo "$in is singular, can't be inverted" 1>&2
    exit 1
fi
zmu $in $iv ${tmp}.1
ret=$?
if [ 0 -ne $ret ]; then
  echo "Failed to multiply ${in} and ${iv}, terminating" 2>&1
  exit $ret
fi
# Produce identity matrix of correct size and check against ${tmp}.1
zid ${tmp}.2 ${prime} ${nor} ${noc}
zdiff ${tmp}.{1,2}
ret=$?
if [ 0 -ne $ret ]; then
  echo "Results differ, failed" 1>&2
  exit $ret
else
  echo "invert worked"
fi
rm -f ${tmp}.{1,2}
