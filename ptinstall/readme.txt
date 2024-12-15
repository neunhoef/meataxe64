Meataxe64 meataxe2000 hybrid release

This is release 1 of the above, dated 20/05/2018
meataxe64 is copyright (c) 2018 Richard Parker
meataxe2000 and the hybrid system is copyright (c) 2018 Jon Thackray

In the same directory as this readme is a script install.sh, which
will build the entire system into a directory of your choice, eg

./install.sh <my meataxe system>

It will then prompt you to extend your path. You can then start using
meataxe2000 scripts with the underlying power of meataxe64. There are
3 possible modes of use: pure meataxe2000, pure meataxe64 and the
hybridised version including both. meataxe2000 has scripts to drive
it, which are also available in the hybrid, but it is restricted (at
present) to fields of order 2, 3, 4, 5, 9. These scripts are also
available in the hybrid version, with the same restrictions (at
present). In pure meataxe64 there are no scripts; you need to invoke
the binaries you need. This can make chopping up a module very long
and laborious. The PATH extensions required are:-
meataxe2000: <my meataxe system>/m2000
meataxe64: <my meataxe system>/ptinstall/bin
hybrid: <my meataxe system>/pinstall:<my meataxe system>/ptinstall/jif:<my meataxe system>/m2000

There are also tests (run_all.sh) for meataxe64 in jif. To run these
you will need <my meataxe system>/ptinstall/bin on your PATH, but none
of the other directories.

Note that meataxe64 has a parameters file git/meataxe64/src/tuning.h
In this you can configure how much memory and how many threads you
want meataxe64 to use, as well as how many pieces you'd like it to
chop into internally. The defaults are set for an 8 thread 128Gb
desktop machine.

For a machines with many more threads and memory, you may wish to
change these. See meataxe64's readme files.

Jon Thackray - jgt@pobox.com
Richard Parker - richpark54@hotmail.co.uk
