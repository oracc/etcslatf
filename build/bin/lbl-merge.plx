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
my $tlit = $lbl; $tlit =~ s/^lbl\+/tlit/; $tlit =~ s/lbl$/atf/;
my $out = $lbl; $out =~ s/^lbl\+/atf/; $out =~ s/lbl$/atf/;

my %lbl = (); load_lbl();

my $tr = undef;

open(TLIT, $tlit) || die;
open(OUT, ">$out") || die;
while (<TLIT>) {
    if ($lbl{$.}) {
	$tr = $lbl{$.};
    } elsif (/^([^\$\#\@].*?\.)\s/) {
	warn "$tlit:$.: no entry in labels for MTS $1\n";
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

1;

################################################################################

sub load_lbl {
    open(L, $lbl) || die;
    while (<L>) {
	chomp;
	next if /^[\$\{]/ || /^\s*$/;
	if (/^(.*?)=/) {
	    my $atf = $1;
	    s/^.*?\t//;
	    $lbl{$atf} = $_;
	}
    }
    close(L);
}
