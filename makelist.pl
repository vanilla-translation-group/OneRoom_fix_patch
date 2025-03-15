#!/usr/bin/env perl

use Encode;

open(my $fh, ">", "filelist.dat") or die "Failed to open file";

my $p = shift, $c = 0, $t = 0, @a = ();

while ($p) {
	$p .= "\n";
	if ($p =~ /[^\x0-\x7f]/) {
		$t += 1 << $c;
		push @a, encode("UCS-2LE", decode("UTF-8", $p));	
	} else {
		push @a, $p;
	}
	if ($c > 6) {
		print $fh chr($t);
		foreach $i (@a) {
			print $fh $i;
		}
		@a = ();
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
