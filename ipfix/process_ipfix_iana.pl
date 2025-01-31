#!/usr/bin/env perl

use Text::CSV qw( csv );
use Data::Dumper;
use strict;

print <<'EOF';
#ifndef IPFIX_IANA_H
#define IPFIX_IANA_H

EOF

my $icmp_hash = csv (in => "icmp-parameters-types.csv",
                     headers => "auto");   # as array of hash

my $flowend_hash = csv (in => "ipfix-flow-end-reason.csv",
                        headers => "auto");   # as array of hash

my $proto_hash = csv (in => "protocol-numbers-1.csv",
                      headers => "auto");   # as array of hash

my $ipfix_hash = csv (in => "ipfix-information-elements.csv",
                      headers => "auto");   # as array of hash

#enum IANA_ABSTRACT_TYPE {
#  IANA_BASICLIST,
#  IANA_BOOLEAN,
#  IANA_DATETIMEMICROSECONDS,
#  IANA_DATETIMEMILLISECONDS,
#  IANA_DATETIMENANOSECONDS,
#  IANA_DATETIMESECONDS,
#  IANA_FLOAT64,
#  IANA_IPV4ADDRESS,
#  IANA_IPV6ADDRESS,
#  IANA_MACADDRESS,
#  IANA_OCTETARRAY,
#  IANA_SIGNED32,
#  IANA_STRING,
#  IANA_SUBTEMPLATELIST,
#  IANA_SUBTEMPLATEMULTILIST,
#  IANA_UNSIGNED8,
#  IANA_UNSIGNED16,
#  IANA_UNSIGNED32,
#  IANA_UNSIGNED64,
#  IANA_UNSIGNED256
#};

#foreach my $row (@$aoh) {
#  print %$row{'Abstract Data Type'} . "\n";
#}

#print Dumper($aoh);

print <<'EOF';

#endif // IPFIX_IANA_H
EOF
