#!/bin/sh
rm -fr tra ; mkdir -p tra
for a in aac/*.xml ; do
    b=`basename $a .xml`
    xsltproc etc/etcsl-tra-atf.xsl $a >tra/$b.atf
done
bin/qrename.plx
