#!/bin/bash
# Timing tests mod 5 and 7
set -x
~/git/m64/bin/zrd m100_5 5 100000 100000 3
time ~/git/m64/bin/zrn m100_5
time ~/git/m64/bin/zec m100_5 rs cs mul cln rem
time ~/git/m64/bin/zmu m100_5 m100_5 foo
~/git/m64/bin/zrd m100_7 7 100000 100000 3
time ~/git/m64/bin/zrn m100_7
time ~/git/m64/bin/zec m100_7 rs cs mul cln rem
time ~/git/m64/bin/zmu m100_7 m100_7 foo
exit 0
~/git/m64/bin/zrd m200_5 5 200000 200000 3
time ~/git/m64/bin/zrn m200_5
time ~/git/m64/bin/zec m200_5 rs cs mul cln rem
time ~/git/m64/bin/zmu m200_5 m200_5 foo
~/git/m64/bin/zrd m200_7 7 200000 200000 3
time ~/git/m64/bin/zrn m200_7
time ~/git/m64/bin/zec m200_7 rs cs mul cln rem
time ~/git/m64/bin/zmu m200_7 m200_7 foo
~/git/m64/bin/zrd m300_5 5 300000 300000 3
time ~/git/m64/bin/zrn m300_5
time ~/git/m64/bin/zec m300_5 rs cs mul cln rem
time ~/git/m64/bin/zmu m300_5 m300_5 foo
~/git/m64/bin/zrd m300_7 7 300000 300000 3
time ~/git/m64/bin/zrn m300_7
time ~/git/m64/bin/zec m300_7 rs cs mul cln rem
time ~/git/m64/bin/zmu m300_7 m300_7 foo
~/git/m64/bin/zrd m500_5 5 500000 500000 3
time ~/git/m64/bin/zrn m500_5
time ~/git/m64/bin/zec m500_5 rs cs mul cln rem
time ~/git/m64/bin/zmu m500_5 m500_5 foo
~/git/m64/bin/zrd m500_7 7 500000 500000 3
time ~/git/m64/bin/zrn m500_7
time ~/git/m64/bin/zec m500_7 rs cs mul cln rem
time ~/git/m64/bin/zmu m500_7 m500_7 foo
