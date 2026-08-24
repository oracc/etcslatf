#!/bin/sh
for a in tlit/*.atf ; do
    b=`basename $a .atf`
    bin/extract-lnums.plx $a >lnums/$b.lnum
done
