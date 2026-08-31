# REPO etcslatf

This repo contains Oracc projects to display ETCSL with interlinear
and with paragraph-oriented translation, as well as tools for going
from the original ETCSL TEI files to ATF translation both in paragraph
and in an interlinear form that is then intended for human review.

The repo is divided into three parts: build, paras, and inter: *build*
is where ETCSL ATF is built from ETCSL sources; *paras* is a project,
etcslatf/paras, that displays the ETCSL corpus with the Oracc version
of ETCSL's original paragraph-oriented translations; *inter* is a
project, etcslatf/inter, that displays the ETCSL corpus with the
translation remapped to interlinear line-by-line format.

# EDITING

Before individual interlinear outputs have had an initial review, only
the files in build/tlit (for transliterations) and build/tei (for
translations) should be edited.

After initial review, every interlinear translation will require human
review and correction to the inevitable misalignments of
transliteration and translation.  During this phase, only the files in
inter/00atf should be edited.

After the human review of interlinear translations has been completed,
corrections to the transliteration should continue to be made in
inter/00atf; corrections to the translation must be made in both
inter/00atf (for the interlinear translation) and in build/tra (for
the paragraph translations).

The 00atf files for the project etcsl/paras are rebuilt from the
transliterations from inter/00atf and the paragraphs in build/tra.

# BUILD

The work is done in three general phases which may themselves perform
multiple tasks:

 1) tei2tra: Basic ATF translation creation from the source TEI files
 2) tra2lbp: Basic translations are rewritten as .tsv
 3) lbp2atf: TSV files are broken into lines and the lines inserted into ATF

## tei2tra

The TEI files live in tei/: the versions in the repo have very light
modifications to fix some encoding issues in the originals.  These can
be found by extracting the originals from the OTA repository and
diffing against those in tei/.

The TEI has entities resolved and is written to xml/ from where the
files have <addSpan>...<anchor> converted to containers in aac/.

The aac/ files are then converted to bare ATF translations with an XSL
script, etc/aac-tra.xsl, and renamed to their Q-numbers.

The script bin/tei2tra.sh does the tei/ .. tra/ conversion.

## tra2lbp

The tra files are rewritten by bin/lbpp.plx as .tsv files with the
following columns:

* **Q**: The Q-number.
* **LABEL**: The original translation label for the para.
* **LINES**: The number of transliteration lines the para corresponds to.
* **WORDS**: The expected words-per-line
* **LABELS**: The expansion of the label range from the original label giving the labels to be used for each line.
* **TEXT**: The para of translation.

The **TEXT** field is modified to replace ' (?)' with Ctrl-Q and to
bracket ignorable segments (like @gap{...}) with Ctrl-X and Ctrl-Y.

The script bin/tra2lbp does the tra/ .. lbp/ conversion.

## lbp2atf

A C program, src/lb.c does the heavy lifting of choosing line breaks
and writes labeled lines in paragraphs with the original ETCSL
paragraph label at the head of the para.

lb also writes a log file to lbl/\*.log and any diagnostics from the
run are saved in lbl/\*.err.

The lbl/ data is then processed to create merged interlinear translation
from the lbl/ data and the tlit/ data. The tlit/ data is the set of
ATF transliterations derived from ePSD2.

* Tables of line numbers are created using bin/lnums.sh and saved in
  lnum/. This only needs to be done once unless there are changes to
  the tlit/ data.
* The lbl/ data is rewritten so that each labeled line has the tlit/
  data file line number (from lnum/) prepended to the line label; this
  is written to lbl+/.
* The lbl+/ data is merged into the tlit/ data based on
  the line numbers and written to 00atf/.

The script lbp2atf.sh does the lbl/ .. 00atf/ conversion; if lnum/
does not exist lbl2atf.sh calls lnums.sh to create the lnum/ data.

### lnum/

The lnum/ directory contains mostly two-column tables of physical line
numbers in the transliteration file and the label line number.

The first line gives the Q-number and the ETCSL text-id.

All remaining lines that do not start with '$' give the ATF file line
number (i.e., the number of the line in the file, not the manuscript
line number given at the start of the line) and the ETCSL line-id.

Lines starting with '\$' either give an extent in column one along with
the '\$' and a type in column two, either "blank", "fragmentary", or
"blank".  If the ETCSL gap comment contained "approx" this is added in
column three.

If column one has '$$', column two is the literal dollar
line--this is used for $-lines that probably aren't needed for
translation alignment but are kept in the .lnum file just in case.

