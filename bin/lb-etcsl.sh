#!/bin/sh
for a in lbpp/*.tsv ; do
    e=`basename $a .tsv`.err
    echo $a
    src/lb $a 2>lbpp/$e
done
