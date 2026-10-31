#!/usr/bin/env perl

use Encode;

open(my $fh, ">", "filelist.dat") or die "Failed to open file";
binmode $fh;

my $p = shift, $c = 0, $t = 0, @a = ();

while ($p) {
	$p .= "\n";
	if ($p =~ /[^\x0-\x7f]/) {
		$t += 1 << ($c % 8);
		push @a, encode("UCS-2LE", decode("UTF-8", $p));	
	} else {
		push @a, $p;
	}
	if ($c % 8 == 7) {
		print $fh chr($t);
		foreach $i (@a) {
			print $fh $i;
		}
		@a = ();
		$t = 0;
	}
	$c++;
	$p = shift;
}

if (@a) {
	print $fh chr($t);
	foreach $i (@a) {
		print $fh $i;
	}
}

close($fh) or die "Failed to close file";
