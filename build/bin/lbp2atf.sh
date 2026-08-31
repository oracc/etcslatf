#!/bin/sh
if [ ! -d lnum/ ]; then
    bin/lnums.sh
fi
for a in lbl/*.tsv ; do
    bin/lbl2atf-one.sh $a
done
