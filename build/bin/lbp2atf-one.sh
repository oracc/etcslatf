#!/bin/sh
tsv=$1
b=`basename $tsv .tsv`
err=lbp/$b.err
lbl=lbl/$b.lbl
#lblplus=lbl+/$b.lbl
lla=lla/$b.lla
echo --src/lb $tsv
src/lb $tsv >$lbl
#echo --bin/add-lnums.plx $lbl
#bin/add-lnums.plx $lbl
#echo --bin/lbl-merge.plx $lblplus
#bin/lbl-merge.plx $lblplus
echo --bin/lnum-align.plx $lbl
bin/lnum-align.plx $lbl
echo --bin/lla-merge.plx $lla
bin/lla-merge.plx $lla
