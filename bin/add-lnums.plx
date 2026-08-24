#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

# Read the .eid table and add the ATF transliteration line labels to
# the ETCSL translation line labels in the .lbl files.  Write the .lbl
# to the lbl+/ directory.

my $lbl = shift @ARGV;
my $out = $lbl; $out =~ s/^lbl/lbl+/;
my $eid = $lbl; $eid =~ s/^lbl/lnums/; $eid =~ s/lbl$/lnum/;

# initialize paragraph labelling lists using the ATF line number
# before the equals that serves to key the paragraph-based
# translations to the transliteration
my @parlabels = `grep ^'{' $lbl`; chomp @parlabels;
my %p = (); foreach (@parlabels) { /^\{(.*?)\s+=/; @{$p{$1}} = (); }; load_eid();

my $curr_e = undef;
my @elabels = ();
my $elindex = 0;
my $ewarned = 0;

open(L, $lbl) || die;
open(O, ">$out") || die "failed to open '$out' for writing\n";
while (<L>) {
    if (/^\{(.*?)\s+=/) { # para label
	$curr_e = $1;	
	@elabels = @{$p{$curr_e}};
	$elindex = 0;
	$ewarned = 0;
    } else {
	if (/^(.*?)\./) { # labeled translation line
	    if ($elindex <= $#elabels) {
		s/^/$elabels[$elindex++]=/;
	    } else {
		warn "$lbl:$.: more lines than labels\n"
		    unless $ewarned++;
	    }
	}
    }
    print O;
}
close(O);
close(L);

1;

################################################################################
#
# Create a list of line numbers for each para label key
sub load_eid {
    open(E,$eid) || die;
    while (<E>) {
	next if /^[Q\$]/;
	chomp;
	my($atf,$etcsl) = /^(.*?)\s+(.*?)$/;
	if ($p{$etcsl}) {
	    $curr_e = $etcsl;
	}
	die "$eid: $etcsl not in parlabels\n"
	    unless $curr_e;
	push @{$p{$curr_e}}, $atf;
    }
    close(E);
}
