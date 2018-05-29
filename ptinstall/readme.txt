Meataxe64 meataxe2000 hybrid release

This is release 1 of the above, dated 20/05/2018
meataxe64 is copyright (c) 2018 Richard Parker
meataxe2000 and the hybrid system is copyright (c) 2018 Jon Thackray

In the same directory as this readme is a script install.sh, which
will build the entire system into a directory of your choice, eg

./install.sh <my meataxe system>

It will then prompt you to extend your path. You can then start using
meataxe2000 scripts with the underlying power of meataxe64. If you
only want to use meataxe64 you should use the path <my meataxe
system>/bin instead.

Note that meataxe64 has a parameters file src/tuning.h In this you can
configure how much memory and how many threads you want meataxe64 to
use, as well as how many pieces you'd like it to chop into internally.
The defaults are set for an 8 thread 32Gb desktop machine.

For a machine such as Omega, with many more threads and memory, you
may wish to change these. See meataxe64's readme.

Jon Thackray - jgt@pobox.com
Richard Parker - richpark54@hotmail.co.uk
