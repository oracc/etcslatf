#!/bin/sh
for a in lbpp/*.tsv ; do
    e=`basename $a .tsv`.err
    l=`basename $a .tsv`.lbl
    echo $a
    src/lb $a >lbl/$l 2>lbpp/$e
done
