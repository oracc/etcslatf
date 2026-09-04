#!/usr/bin/perl
use warnings; use strict; use open 'utf8'; use utf8; use feature 'unicode_strings';
binmode STDIN, ':utf8'; binmode STDOUT, ':utf8'; binmode STDERR, ':utf8';
binmode $DB::OUT, ':utf8' if $DB::OUT;

use Data::Dumper;

use lib "$ENV{'ORACC_BUILDS'}/lib";

use Getopt::Long;

GetOptions(
    );

#
# Read .lnum for a tlit and the .lbl for a tlat and write .lla (lnum-label-aligned)
#

my $lbl = shift @ARGV;
my $out = $lbl; $out =~ s/^lbl/lla/; $out =~ s/lbl$/lla/;
my $lnum = $lbl; $lnum =~ s/^lbl/lnums/; $lnum =~ s/lbl$/lnum/;
my $log = $out; $log =~ s/lla$/log/;

# read the lnums file
my @lnums = (); my @l = `cat $lnum`; chomp @l;
shift @l;
foreach (@l) {
    my @x = split(/\t/,$_);
    if ($x[0] =~ /\$/) {
	push @lnums, [ "$x[0] $x[1]" , -1 ]; # order is EID/MTS-line-label; ATF file-line-number
    } else {
	push @lnums, [ $x[1] , $x[0] ]; # order is EID/MTS-line-label; ATF file-line-number
    }
}

# read the labels file
my @label = (); @l = `cat $lbl`; chomp @l;
my $para = undef;
foreach (@l) {
    next if /^\s*$/ || /^\&/;
    if (/^{(.*?)}/) {
	$para = $1;
    } else {
	tr/\cX\cY//d;
	if (/^\$/) {
	    push @label, [ '' , $_ , $para ];
	} else {
	    /^(.*?)\.\s+(.*?)\s*$/;
	    push @label, [ $1 , "#tr.en: $2" , $para ];
	}
    }
}

# iterate over the lnums and the labels simultaneously; if the next
# label does not match the MTS of the current lnum, see if there is a
# $-line reason for it in the labels

my $frag = 0;
my $ln = 0; my $ln_top = $#lnums;
my $lb = 0; my $lb_top = $#label;
my @tr = ();

open(L, ">$log") || die;
while ($ln <= $ln_top) {
    my $lnn = ${$lnums[$ln]}[0] || '';
    my $lbn = ${$label[$lb]}[0] || '$';
    # lnum_log("start: $lnn [ln=$ln/$ln_top] and $lbn [lb=$lb/$lb_top]\n");
    if ($lnn eq $lbn) { # this can only happen when both lnn and lbn are EIDs
	lnum_log("$lnn == $lbn\n");
	push @tr, [ ${$lnums[$ln]}[1] , ${$label[$lb]}[1] ];
	$frag = 0;
	++$lb;
    } elsif ($frag > 0) {
	# don't test $lbn in this block, because $lbn is held at the
	# line after the $-line that resulted in non-zero $frag
	if ($lnn =~ /^\$/) { # dollar-line in translit
	    if ($lnn =~ /frag/) {
		my($lnnf) = ($lnn =~ /(\d+)/);
		$frag -= $lnnf;
		if ($frag < 0) {
		    $frag = 0;
		    lnum_log("$lnn when frag=$frag resulted in frag underflow\n");
		} else {
		    lnum_log("$lnn => frag=$frag\n");
		}
	    } else {
		lnum_log("Found '$lnn' while frag=$frag; resetting frag\n");
		$frag = 0;
	    }
	} else { # EID in translit corresponding to fragmentary run in tlat
	    lnum_log("$lnn ++=> fragmentary [frag=$frag]\n");
	    push @tr, [ ${$lnums[$ln]}[1] , '($fragmentary$)' ];
	    --$frag;
	}
    } elsif ($lnn !~ /^\$/) { # lnn is an EID
	if ('$' eq $lbn) {
	    my $lbc = ${$label[$lb]}[1];
	    if ($lbc =~ /fragmentary/) {
		lnum_log("$lnn => fragmentary\n");
		push @tr, [ ${$lnums[$ln]}[1] , '($fragmentary$)' ];
		$frag = need_frag($lbc, '');
		++$lb; # unless $frag > 0;
	    } else {
		lnum_log("EID/\$ mismatch: $lnn vs $lbc\n");
		# could try some resync here
		++$lb;
	    }
	} else {
	    lnum_log("EID/EID mismatch: $lnn != $lbn\n");
	    # lnn and lbn are both EIDs but they don't match; this is where resync could go
	}
    } else { # lnn is a $-line
	if ('$' eq $lbn) {
	    my $lbc = ${$label[$lb]}[1];
	    if ($lnn =~ /fragmentary/ && $lbc =~ /fragmentary/) {
		lnum_log("$lnn ~~ $lbc [frag=$frag]\n");
		$frag = need_frag($lbc, $lnn);
		++$lb; # unless $frag > 0;
	    } elsif ($lnn =~ /missing/ && $lbc =~ /missing/) {
		lnum_log("$lnn ~~ $lbc\n");
		$frag = 0;
		++$lb;
	    } else {
		lnum_log("lnn\$: $lnn !~ $lbc\n");
		$frag = 0;
		++$lb;
	    }
	} else {
	    lnum_log("\$/EID mismatch $lnn != label $lbn\n");
	    ++$lb;
	    ## ($ln,$lb) = resync($ln,$lb);
	}
    }
    ++$ln;
}
close(L);

print_tr();

1;

################################################################################

sub lnum_log {
    print L @_;
}

sub need_frag {
    my ($lbf) = ($_[0] =~ /(\d+)/);
    my $lnf = 0;
    if ($_[1]) {
	$_[1] =~ /(\d+)/;
	$lnf = $1;
    }
    if ($lnf) {
	if ($lbf > $lnf) {
	    return $lbf - $lnf;
	} else {
	    return 0;
	}
    } else {
	return $lbf - 1;
    }
}

sub print_tr {
    open(O,">$out"); select O;
    foreach my $tr (@tr) {
	print "$$tr[0]\t$$tr[1]\n";
    }
    close(O);
}
