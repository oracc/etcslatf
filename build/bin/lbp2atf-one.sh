#!/bin/sh
tsv=$1
b=`basename $tsv .tsv`
err=lbp/$b.err
lbl=lbl/$b.lbl
echo --src/lb $tsv
src/lb $tsv >$lbl
echo --bin/lbl-merge.plx $lbl
bin/lbl-merge.plx $lbl
