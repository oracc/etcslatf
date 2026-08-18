#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my %qe = (); my @qe = `cat etc/q-etcsl.tsv`; chomp @qe;
foreach (@qe) { my($q,$e)=split(/\t/,$_); $e =~ s/;.*//; $e =~ s/^etcsl://; $qe{$q}=$e; }

my @a = <tlit/*.atf>;
foreach (@a) {
    my($q) = (m#tlit/(.*?)\.atf#);
    warn "no $q in etc/q-etcsl.tsv\n" unless $qe{$q};
    my $a = "00atf/$q.atf";
    system 'cp', $_, $a;
    my $t = "tra/t.$qe{$q}.atf";
    if (-r $t) {
	open(T,">>$a");
	print T "\n\@translation labeled en project\n\n";
	close(T);
	system "cat $t >>$a";
    } else {
	warn "no tra/t.$qe{$q}.atf\n"
	    unless $qe{$q} =~ /^0\./;
    }
}

1;

################################################################################
