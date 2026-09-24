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
my $lnum = $lbl; $lnum =~ s/^lbl/lnums/; $lnum =~ s/lbl$/lnum/;
my $tlit = $lbl; $tlit =~ s/^lbl/tlit/; $tlit =~ s/lbl$/atf/;
my $out = $lbl; $out =~ s/^lbl/atf/; $out =~ s/lbl$/atf/;

my %seen = ();
my %lbl = (); load_lbl();
my %lnm = (); load_lnm();

my $tr = undef;

open(TLIT, $tlit) || die;
open(OUT, ">$out") || die;
while (<TLIT>) {
    if ($lnm{$.}) {
	$tr = $lnm{$.};
    } elsif (/^([^\$\#\@].*?\.)\s/) {
	# warn "$tlit:$.: no entry in labels for MTS $1\n";
    }
    print OUT;
    if (/^\#lem/) {
	if ($tr) {
	    print OUT "#tr.en: $tr\n";
	    $tr = undef;
	}
    }
}
close(OUT);
close(TLIT);

foreach my $eid (sort keys %lbl) {
    warn "$tlit: $eid occurs in .lbl but not used in tlit\n"
	unless $seen{$eid};
}

1;

################################################################################

sub load_lbl {
    open(L, $lbl) || die;
    while (<L>) {
	chomp;
	next if /^[\$\{\&]/ || /^\s*$/;
	my($ln,$tr) = (/^(\S+?)\.\s+(.*?)$/);
	$tr =~ tr/\cX\cY//d;
	$lbl{$ln} = $tr;
    }
    close(L);
}

sub load_lnm {
    open(L, $lnum) || die "failed to open $lnum\n";
    <L>; # discard Q-header
    while (<L>) {
	chomp;
	next if /^[\$\{\&]/ || /^\s*$/;
	my($fl,$ln) = (/^(\d+)\s+(.*?)$/);
	if ($lbl{$ln}) {
	    $lnm{$fl} = $lbl{$ln};
	    ++$seen{$ln};
	} else {
	    # warn "$tlit:$ln: no translation\n";
	}
    }
    close(L);
}
