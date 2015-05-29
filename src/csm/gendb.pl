#!/usr/bin/perl

# Web Polygraph       http://www.web-polygraph.org/
# Copyright 2003-2011 The Measurement Factory
# Licensed under the Apache License, Version 2.0

$cdbsize = 5000000; # 5mb expected cdb size
#$tmpfsize = 10000; # 10000 bytes minimum for each generated temp file

$blockpart = 256;
$blockfull = $blockpart * 4;

$mintmpfsize = $blockfull;
$maxtmpfsize = 50000;

$junkgen = "./zgen";
$tmpfname = "/tmp/zgen.junk";

$cdbcmd = "./cdb DATABASE add --format verbatim $tmpfname";

$gencmd = "$junkgen $tmpfname $blockfull BLOCKPART TMPFSIZE";

# generate random file size
sub tfsize {
  my $t=0;
  my $num_iter = 5;
  my $fsize = 0;
  $t += 2*rand() for(1..$num_iter);
  $t = abs($t - $num_iter); 
  $fsize = 1 + int($t * $maxtmpfsize / $num_iter);
  if ($fsize < $mintmpfsize) { $fsize = $mintmpfsize; }
  return $fsize;
}


sub gen_db {
    my $dbname = shift;
    my $blocks = shift;

    my $spacestotal = $blocks * $blockpart;
    my $cmd = "rm -f $tmpfname ; $gencmd; $cdbcmd";
    my $runcmd = "";

    my $lastfsize = 0;

    $cmd =~ s/BLOCKPART/$spacestotal/;
    $cmd =~ s/DATABASE/$dbname/;

    print "$runcmd\n";
    
    `touch $dbname`; # should not be needed, but...
    
    for (my $total=0; $total < $cdbsize; $total += $lastfsize)
    {
        $lastfsize = tfsize();
        $runcmd = $cmd;
        $runcmd =~ s/TMPFSIZE/$lastfsize/;
        print $runcmd."\n";
	
	`$runcmd`;
    }
}

# file name compXX.cdb means that XX% of oroginal files is compressible.
# .cdb files themselves have slightly better compresion ratio

gen_db ("comp00.cdb", 0);
gen_db ("comp25.cdb", 1);
gen_db ("comp50.cdb", 2);
gen_db ("comp75.cdb", 3);
gen_db ("comp99.cdb", 4);

`rm -f $tmpfname`;
