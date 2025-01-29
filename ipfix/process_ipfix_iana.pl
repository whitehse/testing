#!/usr/bin/env perl

use Text::CSV qw( csv );
use Data::Dumper;
use strict;

# Read whole file in memory
my $aoh = csv (in => "ipfix-information-elements.csv",
               headers => "auto");   # as array of hash

foreach my $row (@$aoh) {
  print %$row{'Abstract Data Type'} . "\n";
}

#print Dumper($aoh);
