#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my $lla = shift @ARGV;
my $tlit = $lla; $tlit =~ s/^lla/tlit/; $tlit =~ s/lla$/atf/;
my $out = $lla; $out =~ s/^lla/atf/; $out =~ s/lla$/atf/;

my %lla = (); load_lla();

my $tr = undef;

open(TLIT, $tlit) || die;
open(OUT, ">$out") || die; select OUT;
while (<TLIT>) {
    print;
    if ($lla{$.}) {
	print $lla{$.};
#	$tr = $lla{$.};
    }
    # if (/^\#lem/) {
    # 	if ($tr) {
    # 	    print OUT "#tr.en: $tr\n";
    # 	    $tr = undef;
    # 	}
    # }
}
close(OUT);
close(TLIT);

1;

################################################################################

sub load_lla {
    open(L, $lla) || die;
    while (<L>) {
	my($line,$text) = split(/\t/,$_);
	$lla{$line} = $text;
    }
    close(L);
}
