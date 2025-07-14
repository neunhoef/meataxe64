#!/bin/bash
# Timing tests mod 2 and 3
set -x
zrd m100_2 2 100000 100000 3
time zrn m100_2
time zec m100_2 rs cs mul cln rem
time zmu m100_2 m100_2 foo
zrd m100_3 3 100000 100000 3
time zrn m100_3
time zec m100_3 rs cs mul cln rem
time zmu m100_3 m100_3 foo
zrd m200_2 2 200000 200000 3
time zrn m200_2
time zec m200_2 rs cs mul cln rem
time zmu m200_2 m200_2 foo
zrd m200_3 3 200000 200000 3
time zrn m200_3
time zec m200_3 rs cs mul cln rem
time zmu m200_3 m200_3 foo
zrd m300_2 2 300000 300000 3
time zrn m300_2
time zec m300_2 rs cs mul cln rem
time zmu m300_2 m300_2 foo
zrd m300_3 3 300000 300000 3
time zrn m300_3
time zec m300_3 rs cs mul cln rem
time zmu m300_3 m300_3 foo
zrd m500_2 2 500000 500000 3
time zrn m500_2
time zec m500_2 rs cs mul cln rem
time zmu m500_2 m500_2 foo
zrd m500_3 3 500000 500000 3
time zrn m500_3
time zec m500_3 rs cs mul cln rem
time zmu m500_3 m500_3 foo
