# $1 file
# $2 number
# $3 file stem
# $4 memory
# $5 start point
if [ $# -ne 5 ]; then
  echo bad parameters
  exit 1
fi
x=$5
a=${3}_1
b=${3}_2
tmp=tmp${PPID}
memory=$4
while [ $x -lt $2 ]; do
  echo Trying $x
  zsel $1 ${tmp}.0 $x
  rn=`zrn ${tmp}.0 1000`
  ret=$?
  if [ 0 -ne $ret ]; then
    echo rn returns $ret on ${tmp}.0
  else
    if [ $rn -eq 0 ]; then
      echo ${tmp}.0 is zero and so not considered
    else
      sub=`zsp ${tmp}.0 $1.rip.$x.ss $a $b $memory`
      ret=$?
      if [ $ret -eq 0 ]; then
        echo subspace $1.rip.$x.ss of size $sub found for element $x
        exit
      elif [ $ret -eq 2 ]; then
        echo subspace ran out of memory for element $x
#       let memory=`more_memory $memory`
#       continue
      else
        echo subspace failed for element $x with return $ret
      fi
    fi
    rm $1.rip.$x.ss
  fi
  let x=$x+1
done
