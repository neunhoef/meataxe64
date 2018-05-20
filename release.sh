#!/bin/sh
#
# Script to make a release of the meataxe2000 meataxe64 hybrid
#
# No parameters required
rm -rf ~/release*
mkdir ~/release
cd ~/release/
cvs co src
cvs co mtx64
rm -rf `find . -name CVS`
rm -rf `find . -name .cvsignore`
cd mtx64/
rm -rf bld/ jif/ reg/ test/
cd ..
git clone http://schur/jon/meataxe64.git git
cd git/
rm -rf .git
cd
tar cf release.tar release
