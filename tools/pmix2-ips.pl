#!/usr/bin/perl -w

# Web Polygraph       http://www.web-polygraph.org/
# Copyright 2003-2011 The Measurement Factory
# Licensed under the Apache License, Version 2.0

require 5.003;
use strict;

use POSIX;

if (@ARGV == 1 && $ARGV[0] eq '--help') {
	print usage();
	exit;
}

&web2term();

die(&usage()) unless 2 <= @ARGV && @ARGV <= 3;
$SIG{__WARN__} = sub { print(STDERR &usage()); die $_[0] };

my $Quiet = @ARGV == 3;

my ($Bench, $RR, $Id) = @ARGV;
my $Pairs;

exit(&main());

sub main {

	$Pairs = xceil($RR, 400);
	my $Robots = xceil($RR, 0.4);
	$Robots = $Pairs*xceil($Robots, $Pairs);

	my $Servers = ceil($Robots*0.1 + 500);
	$Servers = $Pairs*xceil($Servers, $Pairs);

	my ($rbtX, $rbtY) = &countToB($Robots);
	my ($srvX, $srvY) = &countToB($Servers);

	# must recalc (adjust) again!
	$Robots = $rbtX * $rbtY;
	$Servers = $srvX * $srvY;

	my ($r, $s) = ($Robots/$Pairs, $Servers/$Pairs);

	if (!$Quiet) {
		xprintf("bench:    %6d\n",      $Bench);
		xprintf("req.rate: %6d/sec (actual: %8.2f/sec)\n",
			$RR, $Robots*0.4);
		xprintf("PCs:      %6dpairs\n", $Pairs);

		xprintf("robots:   %6d (%4d/machine)\n",
			$Robots, $r);
		xprintf("servers:  %6d (%4d/machine)\n",
			$Servers, $s);

		xprintf("\n");
		xprintf("rbt_ips:  %20s\n",
			ipRange2Str(1, $rbtX, $rbtY));
		xprintf("srv_ips:  %20s\n",
			ipRange2Str(128+1, 128+$srvX, $srvY));
	}

	die("$0: math went wrong for robots\n") if $r != int($Robots/$Pairs);
	die("$0: math went wrong for servers\n") if $s != int($Servers/$Pairs);

	return 0 unless defined $Id;

	if ($Id =~ /^clts$/) {
		print(ipRange2Str(1, $rbtX, $rbtY));
	}
	elsif ($Id =~ /^srvs$/) {
		print(ipRange2Str(129, 128+$srvX, $srvY));
	}
	elsif ($Id =~ /^clt(\d+)$/) {
		my $id = $1;
		die("there are only $Pairs clients, cannot have $Id client\n") if $id > $Pairs;
		my $step = $rbtX/$Pairs;
		print(ipRange2Str(($id-1)*$step+1, $id*$step, $rbtY));
	}
	elsif ($Id =~ /^srv(\d+)$/) {
		my $id = $1;
		die("there are only $Pairs servers, cannot have $Id server\n") if $id > $Pairs;
		my $step = $srvX/$Pairs;
		print(ipRange2Str(128+($id-1)*$step+1, 128+$id*$step, $srvY));
	} else {
		die("$0: cannot parse `$Id'; expected something like clt1 or srv4\n");
	}

	print("\n"); # if -t STDOUT;

	return 0;
}

sub countToB {
	my ($countTot) = @_;

	my $rangeX = xceil($countTot, 250);
	$rangeX = $Pairs*xceil($rangeX, $Pairs);

	my $rangeY = xceil($countTot, $rangeX);

	return ($rangeX, $rangeY);
}

sub ipRange2Str {
	my ($minX, $maxX, $maxY) = @_;
	return "10.$Bench.$minX-$maxX.1-$maxY";
}

sub xprintf {
	my $fmt = shift;
	#printf(STDERR "$0: $fmt", @_);
	printf("\t$fmt", @_);
}

# try "ceil(700/0.7)" to see why xceil is needed
sub xceil {
	my ($large, $small) = @_;
	my $c = ceil($large/$small);
	return ($c-1)*$small >= $large ? $c-1 : $c;
}

sub web2term {
	my $query = $ENV{QUERY_STRING};
	return unless defined $query;
	open(STDERR, ">&STDOUT");
	print("Content-type: text/plain\r\n\r\n");
	@ARGV = ($query =~ /=([^&]+)/g);
	printf("./pmix2-ips.pl %s\n\n", join(' ', @ARGV));
}

sub usage {
	return "Usage: $0 <bench_id> <requests/sec> [cltId|srvId]\n";
}

