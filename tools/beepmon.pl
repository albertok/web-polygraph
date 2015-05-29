#!/usr/bin/perl -w

# Web Polygraph       http://www.web-polygraph.org/
# Copyright 2003-2011 The Measurement Factory
# Licensed under the Apache License, Version 2.0

require 5.003;
use strict;

my $TheSelect;
my $TheHandler;

package Client;
use strict;
use Fcntl;
use IO::Socket;

my @ConnectingClients = ();

sub new {
        my ($proto, $addr, $sock) = @_;
        die() unless @_ == 2 || @_ == 3;

        my $type = ref($proto) || $proto;
        my $this = {};
        bless ($this, $type);

        $this->{theAddr} = $addr;
		$this->{theSock} = $sock;
		$this->{theBufIn} = '';
		$this->{doReconnect} = ! defined($sock);

		if ($this->{theSock}) {
			$this->noteConnected();
		} else {
			$this->connectLater();
		}

        return $this;
}

sub action {
	my $this = shift;
	return unless defined $this->{theAction};
	my $action = $this->{theAction};
    $this->{theAction} = undef();
	&{$action}($this);
}


sub connect {
	my $this = shift;
	$this->log("connecting...", 3); 

	my $s = $this->{theSock} = new IO::Socket(); 
	$this->mydie($!) unless defined $s;

	my ($remote) = ($this->{theAddr} =~ /^(.*):/);
	my ($port) = ($this->{theAddr} =~ /:(\d+)$/);
	$this->mydie("malformed server address") unless $remote && $port;

	my $iaddr = inet_aton($remote) || $this->mydie("no host: $remote");
	my $paddr = sockaddr_in($port, $iaddr);

	my $proto   = getprotobyname('tcp');
	$s->socket(PF_INET, SOCK_STREAM, $proto) || $this->mydie("socket: $!");

	if (!$s->connect($paddr)) {
		$this->mydie() unless $this->{doReconnect};
		$this->log("connect failed, will retry");
		$this->connectLater();
	} else {
		$this->log("connected", 3);
		$this->noteConnected();
	}
}

sub noteConnected {
	my $this = shift;

	my $s = $this->{theSock} || die();

	$s->print("RPY 0 0 . 0 11\r\n");
	$s->print('<greeting/>');
	$s->print("END\r\n");

	my $start =
		"<start number='1'>\n".
		"   <profile uri='http://www.web-polygraph.org/beep/profiles/basic/' />\n".
		"</start>\n";
	$s->printf("MSG 0 1 . 11 %d\r\n", length($start));
	$s->print($start);
	$s->print("END\r\n");

	fcntl($s, F_SETFL, O_NONBLOCK) || $this->mydie("fcntl: $!");
	$TheSelect->add([$s, $this]);
}

sub noteReadReady {
	my $this = shift;
	my $s = $this->{theSock};

	if (my @newText = $s->getlines()) {
		$this->{theBufIn} .= join('', @newText);
		$this->noteContent();
	} else {
		$TheSelect->remove($s);
		$s->close();
		$this->{theSock} = undef();
		if ($this->{doReconnect}) {
			$this->log("disconnected, will try to reconnect");
			$this->connectLater();
		} else {
			$this->log("disconnected");
		}
	}
}

sub noteContent {
	my $this= shift;
	while ($this->{theBufIn} =~ s|<session\s+([^>]*)/>||m) {
		my $cnt = $&;
		$this->{theBufIn} = $';
		$this->log("received: $cnt", 2);
		$this->handle($cnt);
		chomp($cnt);
		print(STDOUT "$cnt\n");
	}
}

sub handle {
	my ($this, $msg) = @_;
	return unless defined $TheHandler;
	my %savedENV =  %ENV;
	while (length($msg)) {
		$msg =~ s|^[^<]+||;
		while ($msg =~ s~^.*?<(\w+)\s+(\w+)=(?:\'([^\']*)\'|\"([^\"]*)\")\s*~<$1 ~m) {
			$ENV{"$1__$2"} = $3;
			# print("\$ENV{\"$1__$2\"} = $3\n");
		}
		while ($msg =~ s|^.*?<(\w+)\s*/>||m) {
			$ENV{"$1__"} = $1;
			# print("\$ENV{\"$1__\"} = $1;\n");
		}
	}
	system($TheHandler) == 0 or $this->mydie("$TheHandler failed: $!");
	%ENV = %savedENV;
}

sub connectLater {
	my $this = shift;
	$this->{theAction} = \&connect;
	push @Client::ConnectingClients, $this;
}

sub log {
	my ($this, $msg, $lvl) = @_;
	&Log($this->{theAddr}. ": $msg", $lvl);
}

sub mydie {
	my $this = shift;
	$this->log('fatal error: '. join(' ', @_), -1);
	die("\n");
}

1;

use IO::Select;

my $VerbLevel = 1;
my @Clients = ();
my @TheDoormanAddrs = ();
my $TheListenAddr;

&getOpts();

die(&usage()) unless @TheDoormanAddrs || $TheListenAddr;
$SIG{__WARN__} = sub { print(STDERR &usage()); die $_[0] };


exit(&main());

sub main {
	$TheSelect = new IO::Select;

	foreach my $addr (map { (&range2arr($_)) } @TheDoormanAddrs) {
		push @Clients, new Client($addr);
	}
	die() unless @Clients == @Client::ConnectingClients;

	&startServer($TheListenAddr) if $TheListenAddr;

	while (1) {
		&checkIo();
		&checkClients();
		select(undef, undef, undef, 1) unless $TheSelect->count();
	}
}

sub startServer {
	my $addr = shift;

	my $s = new IO::Socket::INET(LocalAddr => $addr, 
		Listen => 1024, Reuse => 1, Proto => "tcp");
    die("failed to listen at $addr: $!\n") unless defined $s;
	fcntl($s, F_SETFL, O_NONBLOCK) || die("fcntl: $!");

	$TheSelect->add([$s, undef()]);

	&Log("listening at $addr", 1);
}

sub checkIo {
	return unless $TheSelect->count();
	&Log("waiting for activity...", 3);
	my $timeout = @Client::ConnectingClients ? 1 : undef();
	foreach my $h ($TheSelect->can_read($timeout)) {
		my ($sock, $clt) = @{$h};
		$clt ? $clt->noteReadReady($sock) : &accept($sock);
	}
}

sub checkClients {
	my @cltsToCheck = @Client::ConnectingClients;
	@Client::ConnectingClients = ();
	foreach my $clt (@cltsToCheck) {
		$clt->action();
	}
}

sub accept {
	my $listSock = shift;
	my $sock= $listSock->accept();
	die("failed to accept a connection at $TheListenAddr: $!\n") unless $sock;
	my $them = sprintf('%s:%d', $sock->peerhost(), $sock->peerport());
	&Log("accepted a connection from $them", 2);

	my $clt = new Client($them, $sock);
	push @Clients, $clt;
}

sub Log {
	my ($msg, $level) = @_;
	$msg .= "\n" unless $msg =~ /\n$/;
	$msg = "$0: $msg" unless $msg =~ /:\s/;
	print(STDERR $msg) if !defined($level) || ($level <= $VerbLevel);
}

sub range2arr {
	my $range = shift;
	my @bins = ();
	while ($range =~ s/([.:])?([^.:]+)//) {
		my $sep = $1 || '';
		my $spec = $2;
		my ($min, $max) = $spec =~ /-/ ?
			($spec =~ /^(\d+)-(\d+)$/) : ($spec =~ /^(\d+)$/);
		return undef unless defined $min;
		$max = $min if !defined($max);
		push @bins, { min=>$min, max=>$max, pos=>$min, sep=>$sep };
	}

	my @res = ();
	while (1) {
		push @res, &curAddr(\@bins);
	} continue {
		last unless nextIter(\@bins);
	}

	return @res;
}

sub nextIter {
	my ($bins, $level) = @_;
	$level = $#{$bins} if !defined $level;
	return undef if $level < 0;

	my $b = $bins->[$level];
	if ($b->{pos} >= $b->{max}) {
		$b->{pos} = $b->{min};
		return &nextIter($bins, $level-1);
	}
	$b->{pos}++;

	return 1;
}

sub curAddr {
	my $bins = shift;
	
	my $addr = '';
	for (my $i = 0; $i <= $#{$bins}; ++$i) {
		my $b = $bins->[$i];
		$addr .= $b->{sep};
		$addr .= sprintf("%d", $b->{pos});
	}
	return $addr;
}

sub getOpts {

	my @newOpts = ();

	for (my $i = 0; $i <= $#ARGV; ++$i) {
		my $opt = $ARGV[$i];

		if ($opt !~ /^--/) {
			push @newOpts, $opt;
			next;
		}

		if ($opt =~ /^--recv_from?$/) {
			my $addr = $ARGV[++$i];
			die("$0: --recv_from requires an argument\n") if !defined($addr);
			push (@TheDoormanAddrs, $addr);
			next;
		}

		if ($opt eq '--listen_at') {
			my $addr = $ARGV[++$i];
			die("$0: --listen_at requires an argument\n") if !defined($addr);
			die("$0: only one --listen option is allowed\n") if defined($TheListenAddr);
			$TheListenAddr = $addr;
			next;
		}

		if ($opt eq '--handler') {
			$TheHandler = $ARGV[++$i];
			die("$0: --handler requires an argument\n") if !defined($TheHandler);
			next;
		}

		if ($opt eq '--verb_lvl') {
			$VerbLevel = $ARGV[++$i];
			die("$0: --verb_lvl requires an integer argument\n") if
				!defined($VerbLevel) || ($VerbLevel !~ /^\-?\d+/);
			next;
		}

		if ($opt eq '--help') {
			print usage();
			exit;
		}

		die("$0: unknown option: $opt\n");
	}

	push (@TheDoormanAddrs, @newOpts);
}

sub usage {
	return "Usage: $0 [--option] ... [recv_from_addr ...]\n\n".
		"Options:\n".
		"  --recv_from <addr(s)>  where to connect to receieve beep messages\n".
		"  --listen_at <addr>     where to listen for beep messages\n".
		"  --handler <cmd>        execute cmd for every message received\n".
		"  --verb_lvl <int>       verbosity level for stderr log\n\n".
		"At least one `recv_from' or `listen_at' address must be given\n";
}
