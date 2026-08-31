#!/bin/sh
for a in lbp/*.tsv ; do
    e=`basename $a .tsv`.err
    l=`basename $a .tsv`.lbl
    echo $a
    src/lb $a >lbl/$l 2>lbp/$e
done
