#!/bin/sh
rm -fr aac
mkdir aac
for a in xml/*.xml ; do
    aac=`basename $a`
    bin/addspan-anchor-containers.plx $a >aac/$aac
done
    
