#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my $lbl = shift @ARGV;
my $out = $lbl; $out =~ s/^lbl/lbl+/;
my $lnum = $lbl; $lnum =~ s/^lbl/lnums/; $lnum =~ s/lbl$/lnum/;

# read the lnums file
my @lnums = (); my @l = `cat $lnum`; chomp @l;
shift @l;
foreach (@l) {
    next if /^\$/;
    my @x = split(/\t/,$_);
    push @lnums, [ $x[1] , $x[0] ]; # order is MTS-line-label; ATF file-line-number
}

# read the labels file
my @label = (); @l = `cat $lbl`; chomp @l;
my $para = undef;
foreach (@l) {
    next if /^\s*$/ || /^\&/;
    if (/^{(.*?)}/) {
	$para = $1;
    } else {
	if (/^\$/) {
	    push @label, [ '' , $_ , $para ];
	} else {
	    /^(.*?)\.\s+(.*?)\s*$/;
	    push @label, [ $1 , $2 , $para ];
	}
    }
}

# iterate over the lnums and the labels simultaneously; if the next
# label does not match the MTS of the current lnum, see if there is a
# $-line reason for it in the labels

my $ln = 0;
my $lb = 0;
my @tr = ();

while ($ln <= $#lnums) {
    my $lnn = ${$lnums[$ln]}[0];
    my $lbn = ${$label[$lb]}[0];
    if ($lnn eq $lbn) {
	warn "comparing $lnn and $lbn\n";
	++$lb;
	++$ln;
	push @tr, [ ${$lnums[$ln]}[1] , ${$label[$lb]}[1] ];
    } else {
	if (!length($lbn)) {
	    my $d = ${$label[$lb]}[1];
	    if ($d =~ /fragmentary/) {
		push @tr, [ ${$lnums[$ln]}[1] , '($fragmentary$)' ];
		warn "$lnn => fragmentary\n";
		++$lb;
	    } else {
		warn "not frag\n";
		++$lb;
	    }
	} else {
	    warn "$lnn != $lbn\n";
	}
	++$ln;
    }
}

1;

################################################################################

