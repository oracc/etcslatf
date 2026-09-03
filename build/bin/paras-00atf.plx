#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my @tlit = <tlit/*.atf>;

foreach my $t (@tlit) {
    mk00atf($t);
}

1;

################################################################################

sub mk00atf {
    my $atf = shift;
    my $tra = $atf;
    $tra =~ s/^tlit/tra/;
    my $out = $atf;
    $out =~ s#^tlit#../paras/00atf#;
    warn "$0: making $out from $atf and $tra\n";
    my $i = 0;
    my @atf = `cat $atf`;
    for ($i = $#atf; $atf[$i] =~ /^\s*$/; --$i) {
	$atf[$i] = '';
    }
    open(O, ">$out") || die; select O;
    for ($i = 0; $i < $#atf; ++$i) {
	if ($atf[$i] =~ /^#project:/) {
	    print "#project: etcslatf/paras\n";
	} elsif (length $atf[$i]) {
	    print $atf[$i];
	}
    }
    if (-r $tra) {
	my @tra = `cat $tra`;
	for ($i = $#tra; $tra[$i] =~ /^\s*$/; --$i) {
	    $tra[$i] = '';
	}
	for ($i = 0; $i <= $#tra; ++$i) {
	    last unless $tra[$i] =~ /^\s*$/;
	}
	print "\n\@translation labeled en project\n\n";
	while ($i <= $#tra) {
	    print $tra[$i++];
	}
    }
    close(O);
}
