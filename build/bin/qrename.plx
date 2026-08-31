#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my %eq = (); my @eq = `cat etc/q-etcsl.tsv`; chomp @eq;
foreach (@eq) { my($q,$e)=split(/\t/,$_); $e =~ s/;.*//; $e =~ s/^etcsl://; $eq{$e}=$q; }

my @x = <tra/*.atf>;
foreach (@x) {
    my($e) = (m#tra/t\.(.*?)\.atf$#);
    warn "no $e in etc/q-etcsl.tsv\n" unless $eq{$e};
    rename $_, "tra/$eq{$e}.atf";
}

1;

################################################################################
