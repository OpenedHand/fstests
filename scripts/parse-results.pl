#!/usr/bin/perl

my $file = shift or die "usage $0 <filename>";

my $runs_per_test   = 10;
my $cur_kbsec_total = 0;
my $cur_ms_total    = 0;
my $cur_fsec_total  = 0;
my $header          = "";

open( IN, "<$file") or die "cant open $file";

print "Device, Blit, Orientation, Bandwidth ( Kb/Sec), Surface Creation time ( ms), Frames per sec\n"; 

while(<IN>)
{
    if (/^test-.*$/)
    {
	if (/(\d+)\s+KB\/sec/i)
	{
	    $cur_kbsec_total += $1;
	}
	elsif (/(\d+)\s+ms/i)
	{
	    $cur_ms_total += $1;
	}
	elsif (/(\d+)\s+frames\/sec/i)
	{
	    $cur_fsec_total += $1;
	}

    } else {

	if ($cur_kbsec_total > 0)
	{
	    print " $header, ".($cur_kbsec_total/$runs_per_test).
		",".($cur_ms_total/$runs_per_test).
		",".($cur_fsec_total/$runs_per_test)."\n";
	}
	
	$cur_kbsec_total = 0;
	$cur_ms_total    = 0;
	$cur_fsec_total  = 0;
	unless (/^\*+/ or /^\s+$/)
	{
	    $header = $_ ;
	    chomp($header);
	}
    }
}

if ($cur_kbsec_total > 0)
{
    print " $header, ".($cur_kbsec_total/$runs_per_test).
	",".($cur_ms_total/$runs_per_test).
	",".($cur_fsec_total/$runs_per_test)."\n";
}

close(IN);
