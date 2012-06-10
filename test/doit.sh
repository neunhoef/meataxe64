#!/bin/sh
#
# Parameters that can be altered
#
# lay3.h #define THREADS 8 would make 8-2 = 6 worker threads!
# mtax.h #define CACHELINE 64 is slightly better for pre-bridge chips.
. ./go
set -x
compalls
reg1
tim1
tim2
