This directory contains scripts which will convert meataxe 2000 binary
calls into calls to meataxe64 scripts and binaries. For example, a zmu
script will simply convert throw away the optional command line
parameters and pass the remainder to meataxe 64 zmu. This directory is
earlier on the PATH than ~/install to ensure that even scripts picked
up from ~/install will call replacements where they exist. Earlier on
the path is ~/pinstall which will contain direct meataxe 64
replacements of meataxe 2000 scripts.
ptinstall/bin should contain the meataxe 64 scripts and binaries
eg export PATH="/home/jon/pinstall/:/home/jon/ptinstall/jif:$PATH"
cp -p meataxe binaries ~/ptinstall/bin
