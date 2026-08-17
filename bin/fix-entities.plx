#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my %ent = (
    aacute=>'á',
    aleph=>'ʾ',
    amacr=>'ā',
    c=>'š',
    C=>'Š',
    ccedil=>'ç',
    commat=>'@',
    eacute=>'é',
    ecirc=>'ê',
    emacr=>'ē',
    g=>'ŋ',
    G=>'Ŋ',
    h=>'ḫ',
    H=>'Ḫ',
    icirc=>'î',
    imacr=>'ī',
    Imacr=>'Ī',
    oacute=>'ó',
    ouml=>'ö',
    s4=>'₄',
    t=>'ṭ',
    times=>'×',
    ucirc=>'û',
    umacr=>'ū',
    uuml=>'ü',
    X=>'…',
);
    
my @f = <*.xml>;
foreach my $f (@f) {
    open(F,$f);
    my $x = "x/$f";
    open(X,">$x");
    while (<F>) {
	1 while s/\&(.*?);/&ent($1)/e;
	s/ id=/ xml:id=/g;
	print X;
    }
    close(X);
    close(F);
}

1;

################################################################################

sub ent {
    if ($ent{$_[0]}) {
	return $ent{$_[0]};
    } else {
	warn "unknown entity $_[0]\n";
	return "XYZ";
    }
}
