#!/bin/bash
wd=`dirname $0`
${wd}/val1 my_vg.sh
${wd}/val2 my_vg.sh
${wd}/val3 my_vg.sh
${wd}/val4 my_vg.sh
${wd}/val5 my_vg.sh
rm a1ch* a2ch* a{1,2,3,4,5,6,7,8,9} junk x xxx y z6
