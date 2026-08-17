#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my %eq = (); my @eq = `cat q-e.tsv`; chomp @eq;
foreach (@eq) { my($q,$e)=split(/\t/,$_); $e =~ s/;.*//; $e =~ s/^etcsl://; $eq{$e}=$q; }

my @x = <x/*.xml>;
foreach (@x) {
    my($e) = (m#x/t\.(.*?)\.xml$#);
    warn "no $e in q-e.tsv\n" unless $eq{$e};
    system "xsltproc -o t/$eq{$e}=${e}_tr.atf tra.xsl $_\n";
}

1;

################################################################################
