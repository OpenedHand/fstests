#!/usr/bin/perl

my $file = shift or die "usage $0 <filename>";

open( IN, "<$file") or die "cant open $file";

@headers = split(/,/,<IN>);

printf('<table frame="all">
        <title>%s</title>
        <tgroup cols="%i" align="char" charoff="50" char=".">
        <thead>
        <row>', $file, scalar(@headers));

foreach $row (@headers)
{
    print "<entry>$row</entry>\n";
}

print "</row>
       </thead>
       <tbody>";

while(<IN>)
{
    print "<row>\n";
    $i = 0;
    @headers = split(/,/, $_);
    for $row (@headers)
    {
	if ($i > 2)
	{
	    printf("<entry align='right'>%.2f</entry>\n", $row);
	} else {
	    print "<entry>$row</entry>\n";
	}
	$i++;
    }
    print "</row>\n";
}

print "</tbody>
       </tgroup>
       </table>";

