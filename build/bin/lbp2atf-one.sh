#!/bin/sh
tsv=$1
b=`basename $tsv .tsv`
err=lbp/$b.err
lbl=lbl/$b.lbl
lblplus=lbl+/$b.lbl
echo src/lb $tsv
src/lb $tsv >$lbl 2>$err
echo bin/add-lnums.plx $lbl
bin/add-lnums.plx $lbl
echo bin/lbl-merge.plx $lblplus
bin/lbl-merge.plx $lblplus
