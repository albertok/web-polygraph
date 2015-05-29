#!/usr/bin/perl -w

# Web Polygraph       http://www.web-polygraph.org/
# Copyright 2003-2011 The Measurement Factory
# Licensed under the Apache License, Version 2.0

require 5.003;
use strict;

if (@ARGV == 1 && $ARGV[0] eq '--help') {
	print usage();
	exit;
}

use POSIX;

&web2term();

# default workload configuration
my $CltSide = {
	max_host_load => 500, # maximum host load
	max_agent_load => 0.4,
	y_octet_offset => 0,
};

my $SrvSide = {
	max_host_load => $CltSide->{max_host_load},
	max_agent_load => $CltSide->{max_host_load},
	y_octet_offset => 128,
};

my $OptRoute = 0;

&getOpts();

die(&usage()) unless 2 <= @ARGV && @ARGV <= 3;
$SIG{__WARN__} = sub { print(STDERR &usage()); die $_[0] };

my $Quiet = @ARGV == 3;

my ($Bench, $RR, $Id) = @ARGV;


exit(&main());

sub main {

	&compute($CltSide);
	&compute($SrvSide);

	#die("$CltSide->{theHostCount} != $SrvSide->{theHostCount}") if
	#	$CltSide->{theHostCount} != $SrvSide->{theHostCount};
	#my $Pairs = $CltSide->{theHostCount};

	if (!$Quiet) {
		my $Robots = scalar @{$CltSide->{theIps}};
		my $Servers = scalar @{$SrvSide->{theIps}};

		xprintf("bench:    %6d\n",      $Bench);

		xprintf("req.rate: %6d/sec (actual: %8.2f/sec)\n",
			$RR, $Robots*$CltSide->{max_agent_load});
		xprintf("PCs:      %6d clients + %d servers (%d total)\n", 
			$CltSide->{theHostCount}, $SrvSide->{theHostCount},
			$CltSide->{theHostCount} + $SrvSide->{theHostCount});

		# xprintf("robots:   %6d (%4d/machine)\n", $Robots, $Robots/$CltSide->{theHostCount});
		# xprintf("servers:  %6d (%4d/machine)\n", $Servers, $Servers/$SrvSide->{theHostCount});

		xprintf("\n");

		printSide('clt', $CltSide);
		xprintf("\n");

		printSide('srv', $SrvSide);
		xprintf("\n");
	}

	return 0 unless defined $Id;

	if ($Id =~ /^clts$/) {
		print(&getIps($CltSide));
	}
	elsif ($Id =~ /^srvs$/) {
		print(&getIps($SrvSide));
	}
	elsif ($Id =~ /^clt(\d+)$/) {
		my $id = $1;
		die("there are only $CltSide->{theHostCount} clients, cannot have $Id client\n") if $id > $CltSide->{theHostCount};
		print(&getIps($CltSide, $id-1));
	}
	elsif ($Id =~ /^srv(\d+)$/) {
		my $id = $1;
		die("there are only $
SrvSide->{theHostCount} servers, cannot have $Id server\n") if $id > $SrvSide->{theHostCount};
		print(&getIps($SrvSide, $id-1));
	} else {
		die("$0: cannot parse `$Id'; expected something like clt1 or srv4\n");
	}

	print("\n");

	return 0;
}

sub printSide {
	my ($label, $side) = @_;

	printf("\t%s.%-20s %10s\n", $label, 'max_host_load:', $side->{max_host_load});
	printf("\t%s.%-20s %10s\n", $label, 'max_agent_load:', $side->{max_agent_load});
	printf("\t%s.%-20s %10s\n", $label, 'subnet:', $side->{theSubnet});
	printf("\t%s.%-20s %10s\n", $label, 'max_addr_per_subnet:', $side->{theMaxAddrPerSnet});
	printf("\t%s.%-20s %10s\n", $label, 'host_count:', $side->{theHostCount});
	printf("\t%s.%-20s %10s\n", $label, 'agent_per_host:', $side->{theAgentPerHost});

	for (my $h = 0; $h <= $#{$side->{thePerHostIps}}; ++$h) {
		printf("\t\t%s.%-20s %s\n", $label, 
			sprintf('%s_%02d.ips:', 'host', $h+1),
			getIps($side, $h));
	}
#	@{$side->{theIps}} = @ips;
}

sub compute {
	my $side = shift;

	my $reqRate = $RR or die();
	my $hostLoad = $side->{max_host_load} or die();
	my $agentLoad = $side->{max_agent_load} or die();

	# find min subnet that can fit maxAddrPerHost addresses
	my $maxAddrPerHost = int($hostLoad/$agentLoad);
	my $subnet = 25;
	my $maxAddrPerSnet = -1;
	for (my $i = 1; $maxAddrPerSnet < $maxAddrPerHost && $i <= 128; $i *= 2) {
		$maxAddrPerSnet = $i * 250;
		--$subnet;
	}
	die() if $maxAddrPerSnet < $maxAddrPerHost;
	die() if $subnet == 25;

	# find the number of agents (i.e. the number of a.b.x.y addresses)
	my $hostCnt = int(xceil($reqRate, $hostLoad));
	my $agentCnt = &doubleDiv($hostCnt, $reqRate, $agentLoad);
	my $agentPerHost = $agentCnt / $hostCnt;
	die() unless $agentCnt <= $maxAddrPerHost * $hostCnt;

	# distribute agentCnt agents among subnets (upto maxAddrPerSnet each)
	my @ips = ();
	$side->{thePerHostIps} = [];
	for (my $s = 0; @ips < $agentCnt; ++$s) {
		# one subnet, one host
		my $host = {};
		my $x = $s * int($maxAddrPerSnet/250);
		my $y = 1;
		for (my $a = 0; $a < $agentPerHost; ++$a) {

			die("request rate is too high; ".
				"ran out of IP addresses while accomodating $agentCnt agents") if $x >= 128;
			my $actualX = $x + $side->{y_octet_offset};
			push @ips, "10.$Bench.$actualX.$y";

			$side->{theFirstX} = $actualX unless defined $side->{theFirstX};
			$side->{theFirstY} = $y unless defined $side->{theFirstY};
			$host->{theFirstX} = $actualX unless defined $host->{theFirstX};
			$host->{theFirstY} = $y unless defined $host->{theFirstY};

			$side->{theLastX} = $host->{theLastX} = $actualX;
			$side->{theLastY} = $host->{theLastY} = $y;

			if (++$y == 251) {
				++$x;
				$y = 1;
			}
		}
		push @{$side->{thePerHostIps}}, $host;
	}
	die() unless $agentCnt == @ips;

	@{$side->{theIps}} = @ips;
	$side->{theSubnet} = $subnet;
	$side->{theMaxAddrPerSnet} = $maxAddrPerSnet;
	$side->{theMaxAddrPerHost} = $maxAddrPerHost;
	$side->{theHostCount} = $hostCnt;
	$side->{theAgentPerHost} = $agentCnt / $hostCnt;

	return undef();
}

sub getIps {
	my ($side, $id) = @_;

	return &formatIps($side, $side->{thePerHostIps}->[$id]) if defined $id;
	return &formatIps($side, $side);
}

sub formatIps {
	my ($side, $descr) = @_;

	if ($OptRoute || $descr->{theFirstX} == $descr->{theLastX} || $descr->{theLastY} == 250) {
		return &ipRange2Str($side, $descr->{theFirstX}, $descr->{theLastX}, 
			$descr->{theLastY});
	} else {
		return sprintf('%s;%s',
			&ipRange2Str($side, $descr->{theFirstX}, $descr->{theLastX}-1, 250),
			&ipRange2Str($side, $descr->{theLastX}, $descr->{theLastX}, $descr->{theLastY}));
	}
}

sub ipRange2Str {
	my ($side, $minX, $maxX, $maxY) = @_ or die();
	my $minY = 1;

	if ($OptRoute) {
		my $subnet = $side->{theSubnet};
		return "10.$Bench.$minX.0/$subnet";
	} else {
		my $x = ($minX == $maxX) ? $minX : "$minX-$maxX";
		my $y = ($minY == $maxY) ? $minY : "$minY-$maxY";
		return "10.$Bench.$x.$y"; # /$subnet";
	}
}

sub doubleDiv {
	my ($factor, $n, $d) = @_;
	my $apx = xceil($n, $d);
	return int($factor * xceil($apx, $factor));
}

# try "ceil(700/0.7)" to see why xceil is needed
sub xceil {
	my ($large, $small) = @_;
	my $c = ceil($large/$small);
	return ($c-1)*$small >= $large ? $c-1 : $c;
}

sub xprintf {
	my $fmt = shift;
	#printf(STDERR "$0: $fmt", @_);
	printf("\t$fmt", @_);
}

sub web2term {
	my $query = $ENV{QUERY_STRING};
	return unless defined $query;
	open(STDERR, ">&STDOUT");
	print("Content-type: text/plain\r\n\r\n");
	@ARGV = ($query =~ /=([^&]+)/g);
	printf("./pmix2-ips.pl %s\n\n", join(' ', @ARGV));
}

sub getOpts {

	my @newOpts = ();

	my $sides = {
		'client' => $CltSide,
		'server' => $SrvSide,
	};

	for (my $i = 0; $i <= $#ARGV; ++$i) {
		my $opt = $ARGV[$i];

		if ($opt !~ /^--/) {
			push @newOpts, $opt;
			next;
		}

		if ($opt eq '--route') {
			$OptRoute = 1;
			next;
		}

		if ($opt =~ /^--(client|server)_side\.(max_(?:host|agent)_load)/) {
			die(&usage()) unless 
				defined $sides->{$1} && defined $sides->{$1}->{$2};
			$sides->{$1}->{$2} = $ARGV[++$i] or die(&usage());
			next;
		}

		die("$0: unknown option: $opt\n");
	}

	@ARGV = @newOpts;
}

sub usage {
	return "Usage: $0 [--option] ... <bench_id> <requests/sec> [cltId|srvId]\n\n".
		"Options:\n".
		"  --route                                 show addresses in route-friendly format\n".
		"  --client_side.max_host_load <req/sec>   load per polyclt process\n".
		"  --server_side.max_host_load <req/sec>   load per polysrv process\n".
		"  --client_side.max_agent_load <req/sec>  load per robot agent\n".
		"  --server_side.max_agent_load <req/sec>  load per server agent\n";
}

sub suffix {
	my ($ord) = @_;
	return 'st' if $ord == 1;
	return 'nd' if $ord == 2;
	return 'rd' if $ord == 3;
	return 'th';
}

