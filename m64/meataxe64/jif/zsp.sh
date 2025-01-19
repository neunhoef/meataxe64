#!/bin/bash
#
# Script to do spin using m64 technology
#
# $1: subspace to be spun
# $2: result stem
# $3: prethrash limit
# $4: subspace limit
# $4-: generators
set -e
#set -x
usage="$0: $0 <seeds> <result stem> <prethrash limit> <spin limit> <generators>"
if [ $# -lt 5 ]; then
    echo $0: insufficient arguments 1>&2
    echo $usage 1>&2
    exit 1
fi
# Work out where binaries and scripts are
pushd `dirname $0` 1>/dev/null
dir=`pwd`/../bin
popd 1>/dev/null
# Set up a temporary filename
tmp=tmp$$
seed=$1
result=$2
plim=$3
slim=$4
shift 4
# $* is now the generators, $# the number of them
# ${tmp}.1 is our seed
cp ${seed} ${tmp}.1
gens=$#
# Put generators in an array
i=1
while [ $i -le ${gens} ]; do
  gen[$i]=$1
  shift
  let i=$i+1
done
# Start the prethrash
cp ${tmp}.1 ${tmp}.2 # Save null vector
COUNTER=1 # Initialise
while [ $COUNTER -lt ${plim} ] ; do # Limit at parameter 2, default 15
  ${dir}/zcn ${tmp}.1 ${tmp}.2 ${tmp}.3 # Glue initial null vector to current
  mv ${tmp}.3 ${tmp}.2 # Make current
  # Now make new images
  i=1
  while [ $i -le $gens ]; do
    ${dir}/zmu ${tmp}.1 ${gen[$i]} ${tmp}.ax.$i # ax1 := ${tmp}.1 *a[i], mult subspace by ith gen
    if [ 1 -eq $i ]; then
      mv ${tmp}.ax.$i ${tmp}.3
    else
      ${dir}/zcn ${tmp}.3  ${tmp}.ax.$i ${tmp}.4
      mv ${tmp}.4 ${tmp}.3 # Overwrite ${tmp}.3 with sum of images
    fi
    let i=$i+1
  done
  mv ${tmp}.3 ${tmp}.1
  let COUNTER=${COUNTER}+1 # and loop
done
# ${tmp}.1 new, ${tmp}.2 old
# Ie, ${tmp}.1 is images to be done, ${tmp}.2 is those done
# Now we echelise the prethrashed stuff, and get reduced generators
${dir}/zpe ${tmp}.2 ${tmp}.piv.1 ${tmp}.rem # Negative echelise images done into bitstring (${tmp}.piv.1) and remnant (${tmp}.rem)
i=1
while [ $i -le $gens ]; do
  ${dir}/zrx ${tmp}.piv.1 ${gen[$i]} ${tmp}.junk ${tmp}.gen.$i # Get rows of a1 not having a bit in bitstring
  let i=$i+1
done
${dir}/zcx ${tmp}.piv.1 ${tmp}.1 ${tmp}.in ${tmp}.out # Get columns of images to be done (${tmp}.in in, ${tmp}.out not in)
${dir}/zmu ${tmp}.in ${tmp}.rem ${tmp}.inrem # Multiply selected columns with remnant
${dir}/zad ${tmp}.inrem ${tmp}.out ${tmp}.inremout # Add this into non selected columns
${dir}/zpe ${tmp}.inremout ${tmp}.bs ${tmp}.newrem # Echelise the result, ${tmp}.bs is bitstring, ${tmp}.newrem is remnant

# Now finish the spin
while ${dir}/zut 4 ${tmp}.bs; do # Main spin loop. Have we finished
  ${dir}/zcx ${tmp}.bs ${tmp}.rem ${tmp}.rem.sel ${tmp}.rem.nonsel # Split previous remnant according to bs into sel and non sel
  ${dir}/zmu ${tmp}.rem.sel ${tmp}.newrem ${tmp}.6 # Multiply sel by remnant of input space
  ${dir}/zad ${tmp}.6 ${tmp}.rem.nonsel ${tmp}.7 # Add this to non sel
  ${dir}/zpc ${tmp}.piv.1 ${tmp}.bs ${tmp}.piv.2 ${tmp}.8 # Combine previous and current pivots into ${tmp}.piv.2
  mv ${tmp}.piv.2 ${tmp}.piv.1 # Replace old pivots with new
  ${dir}/zrr ${tmp}.8 ${tmp}.7 ${tmp}.newrem ${tmp}.rif # Riffle sum and previous space into ${tmp}.rif
  i=1
  while [ $i -le $gens ]; do
    ${dir}/zrx ${tmp}.bs ${tmp}.gen.$i ${tmp}.gen.$i.sel ${tmp}.gen.$i.nonsel # i = 1 case in loop
    mv ${tmp}.gen.$i.nonsel ${tmp}.gen.$i
    ${dir}/zng ${tmp}.gen.$i.sel ${tmp}.gen.$i.sel.neg # Negate ${tmp}.gen.$i.sel into ${tmp}.gen.$i.sel.neg
    ${dir}/zmu ${tmp}.newrem ${tmp}.gen.$i ${tmp}.9
    ${dir}/zad ${tmp}.gen.$i.sel.neg ${tmp}.9 ${tmp}.10
    # Combine all the output, the results of the adds above
    if [ $i -eq 1 ]; then
      mv ${tmp}.10 ${tmp}.combine
    else
      ${dir}/zcn ${tmp}.10 ${tmp}.combine ${tmp}.11
      mv ${tmp}.11 ${tmp}.combine
    fi
    let i=$i+1
  done
  ${dir}/zcx ${tmp}.piv.1 ${tmp}.combine ${tmp}.12 ${tmp}.14
  ${dir}/zmu ${tmp}.12 ${tmp}.rif ${tmp}.13
  ${dir}/zad ${tmp}.13 ${tmp}.14 ${tmp}.15
  ${dir}/zpe ${tmp}.15 ${tmp}.bs ${tmp}.newrem
  mv ${tmp}.rif ${tmp}.rem
  nor=`${dir}/znor ${tmp}.rem`
  if [ 0 -ne ${slim} -a ${slim} -lt ${nor} ]; then
    rm ${tmp}.*
    echo ${nor} exceeds ${slim}
    exit 1
  fi
done # ${tmp}.piv.1 (bits), ${tmp}.rif (remnant) is the answer
cp ${tmp}.piv.1 ${result}.bs
cp ${tmp}.rem ${result}.rem

${dir}/zut 5 ${tmp}.piv.1 # Exit if whole space
ret=$?
rm ${tmp}.*
if [ $ret -ne 0 ]; then
  echo "Whole space"
  exit 1
fi
