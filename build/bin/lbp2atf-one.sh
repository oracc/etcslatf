#!/bin/sh
tsv=$1
err=lbp/`basename $tsv .tsv`.err
lbl=lbl/`basename $tsv .tsv`.lbl
lblplus=lbl+/`basename $tsv .tsv`.lbl
echo src/lb $tsv
src/lb $tsv >$lbl 2>$err
echo bin/add-lnums.plx $lbl
bin/add-lnums.plx $lbl
echo bin/lbl-merge.plx $lblplus
bin/lbl-merge.plx $lblplus
