#!/bin/sh
#
# Master conversion script for ETCSL translations TEI to ATF
#

# 1) remap entities
rm -fr xml ; mkdir -p xml
bin/fix-entities.plx tei xml

# 2) remap addSpan ... anchor to addSpan ... /addSpan;
# use bin/validate-aac.sh to check the XML is well-formed
bin/xml2aac.sh

# 3) convert container version to atf in tra/ directory--this is the
# version that will have line breaks inserted
bin/aac2tra.sh

