#!/bin/sh
for a in tlit/*.atf ; do
    b=`basename $a .atf`
    bin/extract-eids.plx $a >eid/$b.eid
done
