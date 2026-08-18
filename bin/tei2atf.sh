#!/bin/sh
#
# 1) remap entities
rm -fr xml ; mkdir -p xml
bin/fix-entities.plx tei xml
