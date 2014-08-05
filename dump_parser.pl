#!/usr/bin/perl

use strict;

my $dump_path = shift @ARGV;
my $dump;
my @dump;

unless (-f $dump_path) {
	die "$dump_path not found";
}

open(DUMP, $dump_path) or die "cannot open $dump_path";

{
	local $/ = undef;
	$dump = <DUMP>;
}

@dump = split(/\n -+ -+ \n/, $dump);
shift(@dump);

print "digraph {\n";
foreach my $line (@dump) {
	$line =~ s/\s+/ /g;
	$line =~ s/^\s+//;
	my @node = split(/ /, $line);
	print "\t\"$node[3]\" [label=\"$node[1]\\n$node[8]\\n$node[7]\"];\n";
}
foreach my $line (@dump) {
	$line =~ s/\s+/ /g;
	my @node = split(/ /, $line);
	if ($node[4] ne "0000000000000000") {
		print "\t\"$node[3]\" -> \"$node[4]\" [label=\"n0\" arrowhead=\"normal\"];\n";
	}
	if ($node[5] ne "0000000000000000") {
		print "\t\"$node[3]\" -> \"$node[5]\" [label=\"n1\" arrowhead=\"normal\"];\n";
	}
	if ($node[6] ne "0000000000000000") {
		print "\t\"$node[3]\" -> \"$node[6]\" [label=\"ndc\" arrowhead=\"normal\"];\n";
	}
	if ($node[2] ne "0000000000000000") {
		print "\t\"$node[3]\" -> \"$node[2]\" [label=\"parent\" arrowhead=\"normal\"];\n";
	}
}
print "}\n";

