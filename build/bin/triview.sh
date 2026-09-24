#!/bin/sh
bin/lbpp.plx -q $1
bin/lbp2atf-one.sh lbp/$1.tsv
grep -v '^\(#lem\|#etcsl\)' atf/$1.atf | less
