#!/bin/sh
rm -fr tra ; mkdir -p tra
for a in aac/*.xml ; do
    b=`basename $a .xml`
    xsltproc etc/aac-tra.xsl $a >tra/$b.atf
done
bin/qrename.plx
