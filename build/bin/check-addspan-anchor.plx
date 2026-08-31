#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

# Check the output of xsltproc etc/addspan-anchor.xsl xml/*.xml to
# confirm that they addSpan/anchor are not interwoven in the ETCSL
# corpus.  Note they are not necessarily simple pairs, i.e., there can
# be addSpan addSpan anchor anchor.

my @aa = `xsltproc etc/addspan-anchor.xsl xml/*.xml`;
my @to = ();
for (my $i = 0; $i <= $#aa; ++$i) {
    if ($aa[$i] =~ /^addSpan\t(.*?)$/) {
#	print STDERR "pushed $1\n";
	push @to, $1;
    } else {
	my($xid) = ($aa[$i] =~ /\t(.*?)$/);
#	print STDERR "found $xid\n";
	if ($xid eq $to[$#to]) {
	    pop @to;
	} else {
	    warn "$i: xml:id $xid doesn't match $to[$#to]\n";
	}
    }
}

1;

################################################################################

