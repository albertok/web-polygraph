#!/usr/bin/perl -w

# Web Polygraph       http://www.web-polygraph.org/
# Copyright 2003-2011 The Measurement Factory
# Licensed under the Apache License, Version 2.0

use strict;
use warnings;
use Getopt::Long;

sub isImportant($);

my $scope = 'important';
my $precision = '10';

die(&usage()) unless GetOptions(
	'precision=f' => \$precision,
	'scope=s' => \$scope,
	'help' => sub { print usage(); exit; }
	) && ($scope eq 'important' || $scope eq 'all')	&& ($precision >= 0);

# compute diff percentage reporting precision based .00s used by the user
# XXX: does not work for scientific (1e6) precision format
my $accuracy = length($precision) - length(int($precision)); 
$accuracy -= 1 if $accuracy > 0;
my $pctWidth = 5 + $accuracy; # used for percents and labels
my $pctFormat = sprintf('%%%d.%df', $pctWidth-1, $accuracy);

my ($Fname1, $Fname2) = @ARGV or die(&usage());

my @keys = ();
my %hists = ();

my $Meas1 = &load($Fname1);
my $Meas2 = &load($Fname2);

my %seen = ();

my $diffCount = 0;
foreach my $key (@keys) {
	next if exists $seen{$key};
	$seen{$key} = undef();

	next unless ($scope eq 'all') || isImportant($key);

	my $m1 = $Meas1->{$key};
	my $m2 = $Meas2->{$key};

	my $res = undef();
	if (!defined $m1) {
		next if $m2 !~ /^\d/ || $m2 <= 0;
		$res = "uniq $m2 in $Fname2"; # only report used meas
	}
	elsif (!defined $m2) {
		next if $m1 !~ /^\d/ || $m1 <= 0;
		$res = "uniq $m1 in $Fname1"; # only report used meas
	}
	else {
		if ($m1 =~ /^-?\d+(?:.\d*)?$/ && $m2 =~ /^-?\d+(?:.\d*)?$/) { # numbers

			my $diff = $m1 - $m2;
			my $min = $m1 < $m2 ? $m1 : $m2;
			my $max = $m1 < $m2 ? $m2 : $m1;

			next if $min <= 0 && $max <= 0; # undefined or zero
#warn("$m1 $m2");
			if ($min <= 0 && $max > 0) {
				$res = sprintf("%${pctWidth}s %3.2f vs %3.2f", 'inf', $m1, $m2)
			} else {
				my $rdiff = abs($diff/$min); # relative difference

				next if $rdiff < $precision/100; # ignore "minor" differences

				my $diffDir;
				if ($rdiff < 1e-6) { $diffDir = '='; }
				elsif ($m1 > $m2) { $diffDir = '>'; ++$diffCount; }
				else { $diffDir = '<'; ++$diffCount; }

				$res = sprintf("${pctFormat}%% %3.2f %s %3.2f",
					100*$rdiff, $m1, $diffDir, $m2);
			}
		} else { # strings
			my $diffDir;

			if ($m1 eq $m2) { # identical strings
				next if $precision > 1e-6;
				$diffDir = '==';
			} else {
				++$diffCount;
				$diffDir = 'vs';
			}

			$res = sprintf("%${pctWidth}s %6s %2s %6s",
				'str', $m1, $diffDir, $m2);
		}
	}
	printf("%-40s %s\n", "$key:", $res);
}

exit $diffCount ? 1 : 0;

sub load($) {
	my $fname = shift;

	my $meas = {};
	open(OF, "<$fname") or die("cannot read $fname: $!, stopped");
	while (<OF>) {
		if (/(^\S+).hist:$/) {
			loadHistogram($meas, $1, $fname, \*OF);
		} elsif (/:$/) {
			while (<OF>) { last if /^\s*$/; } # skip tables
		}
		elsif (/(^\S+):\s+(\S+)$/) {
			addMeasurement($meas, $1, $2);
		} else {
			warn("$0: skipping unknown line format in $fname:\n$_\n");
		}
	}
	close(OF);
	return $meas;
}

sub loadHistogram($) {
	my $meas = shift;
	my $name = shift;
	my $fname = shift;
	my $OF = shift;

	my $aggr_min;
	my $aggr_max;
	my $aggr_sum = 0;
	my $aggr_count = 0;
	my %bins;

	while (<$OF>) {
		if (/^\s*$/) {
			last;
		} elsif (/^#/) {
			next;
		} elsif (/^\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)$/) {
			my $bin = $1;
			my $min = $2;
			my $max = $3;
			my $count = $4;
			my $contr = $5;
			my $acc = $6;
			my $sum = $bin * $count;

			# remove first and last bin(s) that contributed less than 1%
			next if $acc < 1 || $acc > 99;

			$aggr_min = $min if !defined($aggr_min) || $min < $aggr_min;
			$aggr_max = $max if !defined($aggr_max) || $max > $aggr_max;
			$aggr_sum += $sum;
			$aggr_count += $count;

			$bins{$bin} = 0 unless exists $bins{$bin};
			$bins{$bin} += $count;
		} else {
			warn("$0: skipping unknown line format in $fname:\n$_\n");
		}
	}

	if ($aggr_count > 0) {
		$hists{$name} = undef();

		addMeasurement($meas, "$name.hist.min", $aggr_min);
		addMeasurement($meas, "$name.hist.max", $aggr_max);
		addMeasurement($meas, "$name.hist.mean", $aggr_sum / $aggr_count);
		addMeasurement($meas, "$name.hist.count", $aggr_count);

		my $i = 25;
		my $count = 0;
		foreach my $bin (sort keys %bins) {
			$count += $bins{$bin};
			for (; ($count * 100/ $aggr_count >= $i) && ($i < 100); $i += 25) {
				addMeasurement($meas, "$name.hist.p$i", $bin);
			}
			last if $i >= 100;
		}
	}
}

sub addMeasurement($) {
	my $meas = shift;
	my $key = shift;
	my $value = shift;

	$meas->{$key} = $value;
	push(@keys, $key);
}

sub isImportant($) {
	my $key = shift;

	# errors are always important
	return 1 if $key =~ /err/;

	# .last measurements are not important
	return 0 if $key =~ /\.last$/;

	# not a histogram
	unless ($key =~ /\.hist\./) {
		# .min and .max measurements are not important
		return 0 if ($key =~ /\.min$/) || ($key =~ /\.max$/);

		# .mean blacklisted by .hist.mean
		if ($key =~ /^(.+)\.mean$/) {
			return 0 if exists $hists{$1};
		}
	}

	return 0 if isRareEvent($key);

	return 1;
}

sub isRareEvent($) {
	my $key = shift;

	my @parts = split(/\./, $key);
	my ($name) = @parts;
	my $main_key;

	my @events = qw(basic offered hit miss cachable uncachable fill ims
			reload range head post put abort page ssl ftp
			100_continue proxy_validations auth tunneled
			proxy_validation rep req);
	if (grep {$_ eq $name} @events) {
		@parts[@parts - 1] = 'count';
		my $count_key = join('.', @parts);
		if ((exists $Meas1->{$count_key}) &&
			(exists $Meas2->{$count_key}) &&
			(exists $Meas1->{'xact.started'}) &&
			(exists $Meas2->{'xact.started'})) {
			my $rate1 = $Meas1->{$count_key} / $Meas1->{'xact.started'};
			my $rate2 = $Meas2->{$count_key} / $Meas2->{'xact.started'};
			return 1 if ($rate1 < 0.1) && ($rate2 < 0.1);
		}
	}

	return 0;
}

sub usage {
	return "Usage: $0 [--precision=percent] [--scope=important|all] <f1.lx> <f2.lx>\n".
		"Options:\n".
		"  --precision=percent    difference threshold in percent, default: 10\n".
		"  --scope=important|all  which stats to compare, default: important\n";
}
