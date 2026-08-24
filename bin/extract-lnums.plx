#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my $curr_text;
my $curr_line;

while (<>) {
    if (/^\&(Q\d+)/) {
	$curr_text = $1;
    } elsif (/^#etcsl:/) {
	if (/text-id=(.*?)\s*$/) {
	    print "$curr_text\t$1\n";
	} else {
	    /=(.*?)\s*$/;
	    print "$curr_line\t$1\n";
	}
    } elsif (/^([^\@\$\#].*?\.)\s/) {
	$curr_line = $.;
	# print "$1\n";
    } elsif (/^\$/) {
	if (/(blank|fragmentary|missing)/) {
	    my $type = $1;
	    my $extent = 0;
	    my $approx = "";
	    if (/(\d+)/) {
		$extent = $1;
	    } else {
		$extent = 0;
	    }
	    if (/approx/) {
		$approx = "\tapprox";
	    }
	    print "\$ $extent\t$type$approx\n";
#	    print;
	} else {
	    chomp;
	    warn "$curr_text: $_\n";
	    print "\$\$\t$_\n";
	}	
    }
}

1;

################################################################################

