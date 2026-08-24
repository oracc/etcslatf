# etcslatf

eid/
====

The eid/ directory contains mostly two-column tables of ETCSL IDs for
each text.

The first line gives the Q-number and the ETCSL text-id.

All remaining lines that do not start with '$' give the ATF line
number and the ETCSL line-id.

Lines starting with '$' either give an extent in column one along with
the '$' and a type in column two, either "blank", "fragmentary", or
"blank".  If the ETCSL gap comment contained "approx" this is added in
column three.

If column one has '$$', column two is the literally dollar
line--this is used for $-lines that probably aren't needed for
translation alignment but are kept in the .eid file just in case.


