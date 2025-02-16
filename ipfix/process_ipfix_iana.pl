#!/usr/bin/env perl

use Text::CSV qw( csv );
use Data::Dumper;
use strict;

print <<'EOF';
#ifndef IPFIX_IANA_H
#define IPFIX_IANA_H

EOF

my $ipfix_hash = csv (in => "ipfix-information-elements.csv",
                      headers => "auto");   # as array of hash
 
@$ipfix_hash = @$ipfix_hash[ 1 .. $#$ipfix_hash ];

my $icmp_hash = csv (in => "icmp-parameters-types.csv",
                     headers => "auto");   # as array of hash

my $flowend_hash = csv (in => "ipfix-flow-end-reason.csv",
                        headers => "auto");   # as array of hash

my $proto_hash = csv (in => "protocol-numbers-1.csv",
                      headers => "auto");   # as array of hash

print <<"END";
enum IANA_ABSTRACT_TYPE {
  IANA_TYPE_NONE,
  IANA_TYPE_BASICLIST,
  IANA_TYPE_BOOLEAN,
  IANA_TYPE_DATETIMEMICROSECONDS,
  IANA_TYPE_DATETIMEMILLISECONDS,
  IANA_TYPE_DATETIMENANOSECONDS,
  IANA_TYPE_DATETIMESECONDS,
  IANA_TYPE_FLOAT64,
  IANA_TYPE_IPV4ADDRESS,
  IANA_TYPE_IPV6ADDRESS,
  IANA_TYPE_MACADDRESS,
  IANA_TYPE_OCTETARRAY,
  IANA_TYPE_SIGNED32,
  IANA_TYPE_STRING,
  IANA_TYPE_SUBTEMPLATELIST,
  IANA_TYPE_SUBTEMPLATEMULTILIST,
  IANA_TYPE_UNSIGNED8,
  IANA_TYPE_UNSIGNED16,
  IANA_TYPE_UNSIGNED32,
  IANA_TYPE_UNSIGNED64,
  IANA_TYPE_UNSIGNED256
};

enum IANA_SEMANTICS {
  IANA_SEMANTIC_NONE,
  IANA_SEMANTIC_DELTACOUNTER,
  IANA_SEMANTIC_IDENTIFIER,
  IANA_SEMANTIC_FLAGS,
  IANA_SEMANTIC_DEFAULT,
  IANA_SEMANTIC_QUANTITY,
  IANA_SEMANTIC_TOTALCOUNTER,
  IANA_SEMANTIC_LIST,
  IANA_SEMANTIC_SNMPCOUNTER,
  IANA_SEMANTIC_SNMPGAUGE
};

enum IANA_UNITS {
  IANA_UNIT_NONE,
  IANA_UNIT_OCTETS,
  IANA_UNIT_PACKETS,
  IANA_UNIT_FLOWS,
  IANA_UNIT_BITS,
  IANA_UNIT_SECONDS,
  IANA_UNIT_MILLISECONDS,
  IANA_UNIT_MICROSECONDS,
  IANA_UNIT_NANOSECONDS,
  IANA_UNIT_MESSAGES,
  IANA_UNIT_HOPS,
  IANA_UNIT_ENTRIES,
  IANA_UNIT_4_OCTET_WORDS,
  IANA_UNIT_INFERRED,
  IANA_UNIT_FRAMES,
  IANA_UNIT_PORTS
};
END

#static char axis[3][8] = { "X", "Y", "Z" };

my $ipfix_header = @$ipfix_hash[0];
foreach my $key (keys %$ipfix_header) {
  my $name = lc $key;
  $name  =~ s/ /_/g;
  #print $name."\n";
}

print <<'EOF';
struct iana_ipfix {
  uint32_t elementid; /* 8 */
  char *name; /*sourceIPv4Address */
  int abstract_data_type; /* ipv4Address/IANA_TYPE_IPV4ADDRESS */
  int semantic; /* deltaCounter/IANA_SEMANTIC_DELTACOUNTER */
  int unit; /* octets/IANA_UNIT_OCTETS */
  int low_range;
  int high_range;
};
EOF

my $num_of_ipfix_elements = @$ipfix_hash;
print "static struct iana_ipfix iana_ipfix_elements[$num_of_ipfix_elements] = {\n";

foreach my $ipfix_hash_row (@$ipfix_hash) {
  my $abstract_data_type = "IANA_TYPE_NONE";
  if ($ipfix_hash_row->{'Abstract Data Type'} ne '') {
    $abstract_data_type = "IANA_TYPE_" . uc $ipfix_hash_row->{'Abstract Data Type'};
  }
  my $data_type_semantic = "IANA_SEMANTIC_NONE";
  if ($ipfix_hash_row->{'Data Type Semantics'} ne '') {
    $data_type_semantic = "IANA_SEMANTIC_" . uc $ipfix_hash_row->{'Data Type Semantics'};
  }
  my $unit = "IANA_UNIT_NONE";
  if ($ipfix_hash_row->{'Units'} ne '') {
    $unit = "IANA_UNIT_" . uc $ipfix_hash_row->{'Units'};
    $unit  =~ s/-/_/g;
    $unit  =~ s/ /_/g;
  }
  my $low_range = 0;
  my $high_range = "0xFFFFFF";
  if ($ipfix_hash_row->{'Range'} =~ m/\-/) {
    ($low_range, $high_range) = $ipfix_hash_row->{'Range'} =~ /(.*)\-(.*)/;
  }
print <<"EOF";
  { $ipfix_hash_row->{'ElementID'}, "$ipfix_hash_row->{'Name'}", $abstract_data_type, $data_type_semantic, $unit, $low_range, $high_range},
EOF
}

print <<'EOF';
};
EOF


print <<'EOF';

#endif // IPFIX_IANA_H
EOF
