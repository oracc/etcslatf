#!/bin/sh
if [ ! -d lnum/ ]; then
    bin/lnums.sh
fi
for a in lbp/*.tsv ; do
    bin/lbp2atf-one.sh $a
done
