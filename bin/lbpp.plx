#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;
use integer;

#
# lbpp.plx: line-break pre-processor.
#
# This script parses translation ranges and mangles translation
# paragraphs to facilitate subsequent insertion of likely line
# boundaries.
#
# The output is a table with the following fields:
#
#   Q-number
#   Label
#   Goal line count
#   Expected words per line segment
#   Range of line numbers, expanded from label
#   Translation paragraph as a single string
#
# The translation has had ' (?)' mapped to \cQ
#

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

my $verbose = 0; # print progress messages

GetOptions(
    'v+'=>\$verbose,
    );

my %xranges = (); my @xr = `cat etc/x-ranges.tsv`; chomp(@xr);
foreach (@xr) { my($l,$r) = split(/\t/,$_); $xranges{$l} = $r }

my $file = '';
my $Q = '';
my @q = <tra/*.atf>;
foreach my $q (@q) {
    my $Q = $q; $Q =~ s#^tra/(Q\d+)\.atf#$1#;
    _lbpp($q,$Q);
}

1;

################################################################################

sub _lbpp {
    ($file,$Q) = @_;
    my $L = '';
    open(Q,$file);
    my $b = $file; $b =~ s/tra/lbpp/; $b =~ s/atf$/tsv/;
    open(B,">$b") || die "can't open $b\n";
    while (<Q>) {
	chomp;
	if (s/^\@\((.*?)\)\s+//) {
	    my $label = $L = $1;
	    my($count,@range) = parse_label($label);
	    next if $count == 0;
	    s/\s+\(\?\)/\cQ/g; # map ' (?)' to \cQ 
	    my $rW = (tr/ / /); # raw word count
	    my @b = bifurcate($_);
	    my $nW = words_in_para(@b);
	    my $xW = $nW / $count; # expected Words per line of text
	    if ($xW == 0) {
		$xW = 4;
	    }
	    dump_bifurcated($Q,$label,$count,$rW,$nW,$xW,\@range,\@b);
	}
    }
    close(B);
}

# split a paragraph into content and comment; list members that are
# comments are prefixed by \cX
sub bifurcate {
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
		if ('q' ne $tag && 'fsux' ne $tag && 'varh' ne $tag) {
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
		    # Now $s[$start..$i] is the collection of nodes we don't want
		    push @n, "\cX\@$tag_and_arg".join('',@s[$start..$i]);
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

sub count_words_in_seg {
    my ($p,$v) = @_;
    $p =~ s/\s*--\s*/ /g; # replace ' -- ' with ' '
    $p =~ s/\([^()]+\)[,.!?:]*//g; $p =~ s/^\s+//; $p =~ s/\s+$//; $p =~ s/\s+/ /g;
    my @w = grep defined&&length, split(/\s+/,$p);
    $#w+1;
}

sub dump_bifurcated {
    my($Q,$lab,$cnt,$raw,$num,$exp,$ran,$seg) = @_;
    my @r = @$ran;
    my $str = str_from_segs($Q,$lab,@$seg);
    print B "$Q\t$lab\t$cnt\t$exp\t@r\t$str\n";
}

sub expand_range {
    my ($prefix,$beg,$end) = @_;
    $prefix = '' unless $prefix;
    warn "$0: expand_range prefix=$prefix; beg=$beg; end=$end\n"
	if $verbose;
    my @r = ();
    while ($beg ne $end) {
	push @r, "$prefix$beg";
	if (length("$beg") > 1) {
	    ++$beg;
	    # Perl increments alpha variable Z to AA, AB, AC, etc.; we
	    # only want Z, AA, BB, CC
	    while ("$beg" !~ /^(.)(?:\1)+$/) {
		++$beg;
	    }
	} else {
	    ++$beg;
	}
    }
    push @r, "$prefix$beg";
    warn "$0: range = @r\n"
	if $verbose;
    return @r;
}

sub parse_label {
    my $l = shift;
    my @r = ();
    my $count = 0;
    if ($l =~ /^(.+?)\s+=\s+(.+)$/) {
	my($pre,$pst) = ($1,$2);
	if ($pst =~ /^(.*?)-(.*?)$/) {
	    my ($first,$last) = ($1,$2);
	    warn "$file:$.: label=$pst; first=$first; last=$last\n"
		if $verbose;
	    if ($first =~ /^\d+$/ && $last =~ /^\d+$/) {
		$count = ($last - $first) + 1;
		@r = ($first .. $last);
		warn "$file: number range = @r\n"
		    if $verbose>1;
	    } elsif ($first =~ /^\d*[A-Z]+$/ && $last =~ /^\d*[A-Z]+$/) {
		my ($fdig,$flet) = ($first =~ m/^(\d*)([A-Z]+)$/);
		my ($ldig,$llet) = ($last =~ m/^(\d*)([A-Z]+)$/);
		$fdig = 0 unless $fdig; # A-H is a legal range
		$ldig = 0 unless $ldig;
		if ($fdig == $ldig) {
		    @r = expand_range($fdig,$flet,$llet);
		    $count = $#r + 1;
		} else {
		    # This is an error condition that is not present in original ETCSL trans labels
		    warn "$file:$.: alphanumeric range $pst has differing number portions\n";
		}
	    } else {
		if ($xranges{$pst}) {
		    @r = split(/\s+/, $xranges{$pst});
		    $count = $#r + 1;
		} else {
		    warn "$file:$.: asymmetrical range $pst not in etc/x-ranges.tsv\n";
		}		
	    }
	} else {
	    if ($pst !~ /^\d+$/) {
		# This doesn't matter as long as ax is able to
		# reconcile it as a parallel to a line with the
		# same "number"
		
		# warn "$0: rangeless $pst is not simple number\n";
	    }
	    @r = ($pst);
	    $count = 1;
	}
    }
    ($count,@r);
}

sub str_from_segs {
    my $q = shift;
    my $l = shift;
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

sub words_in_para {
    my @p = @_;
    my $n = 0;
    foreach my $p (@p) {
	next if $p =~ /^\cX/;
	$n += count_words_in_seg($p);
    }
    $n;
}
