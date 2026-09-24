#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

my %drop = (); @drop{qw/varn vari/} = ();
my %keep = (); @keep{qw/fsux gap noteref q wDN wGN wSN/} = ();
my %text = (); @text{qw/varh varl vart/} = ();

while (<>) {
    next unless /^\@\(/;
    my @x = unvar($_);
    print str_from_segs(@x), "\n";
}

sub str_from_segs {
    my $str = '';
    foreach my $seg (@_) {
	if ($seg =~ /^\cX/) {
	    $str .= "$seg\cY";
	} else {
	    $str .= "$seg ";
	}
    }
    $str =~ s/\s+/ /g;
    $str =~ s/\s+$//;
    warn "string has newline\n" if $str =~ /\n/s; # can't happen
    $str;
}

sub unvar {
    my @s = grep defined, split(/([{}])/, $_[0]);
    my @n = ();
    my $curly = 0;
    my @tagless = ();
    for (my $i=0; $i <= $#s; ++$i) {
	if ($s[$i] eq '{') {
	    ++$curly;
	    push @n, $s[$i];
	} elsif ($s[$i] eq '}') {
	    --$curly;
	    push @n, $s[$i];
	} else {
	    if ($s[$i] =~ s/\@([a-z]+(?:\[.*?\])?)$//) {
		my $tag_and_arg = $1;
		if (length($s[$i])) {
		    push @n, $s[$i]; # save the piece up to the tag regardless
		}
		my $tag = $tag_and_arg; $tag =~ s/\[.*$//;
		if (!exists($drop{$tag}) && !exists($keep{$tag}) && !exists($text{$tag})) {
		    warn "unhandled tag $tag\n";
		}
		if (exists($drop{$tag})) {
		    my $endcurly = $curly;
		    my $start = ++$i;
		    while ($i <= $#s) {
			if ('{' eq $s[$i]) {
			    ++$curly;
			} elsif ('}' eq $s[$i]) {
			    --$curly;
			    last if $curly == $endcurly;
			}
			++$i;
		    }
		    # don't push anything onto list for dropped tags
		} elsif (exists($text{$tag})) {
		    my $endcurly = $curly;
		    my $start = ++$i;
		    my @text = ();
		    while ($i <= $#s) {
			if ('{' eq $s[$i]) {
			    ++$curly;
			} elsif ('}' eq $s[$i]) {
			    --$curly;
			    last if $curly == $endcurly;
			}
			++$i;
		    }
		    --$i if $curly; # e.g., para missing the } of a vart
		    # Now $s[$start..$i] is the collection of text children of varh/vart/varl
		    my $n = join('',@s[$start+1..$i-1]);
		    if ($n =~ /\@varl/) {
			my @nx = unvar($n);
			push @n, join('', @nx);
		    } else {
			push @n, $n;
		    }
		} else {
		    # we are keeping these tags
		    push @n, "\@$tag_and_arg";
		}
	    } else {
		push @n, $s[$i];
	    }
	}
    }
    # Now all the dropped nodes should start with \cX
    # Clean the list up by joining sequences of keeper nodes
    my @nn = ();
    for (my $i=0; $i <= $#n; ++$i) {
	warn "$n[$i]\n" if $n[$i] =~ /\@varl/;
	1 while ($n[$i] =~ s/\@varn\{.*?\}//);
	s/ +/ /g;
	push @nn, $n[$i];
    }
    @n = @nn;
    @nn = ();
    for (my $i=0; $i <= $#n; ++$i) {
	if ($n[$i] =~ /^\cX/) {
	    push @nn, $n[$i];
	} else {
	    my $start = $i;
	    while ($i < $#n && $n[$i+1] !~ /^\cX/) {
		++$i;
	    }
	    my $x = join('',@n[$start..$i]);
	    push @nn, $x;
	}
    }
    @nn;
}

1;

################################################################################

