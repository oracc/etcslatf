#!/bin/sh
for a in aac/*.xml ; do
    xmllint --noout $a
done
