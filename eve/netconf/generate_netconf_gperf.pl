#!/usr/bin/env perl

#use Text::CSV qw( csv );
use Data::Dumper;
use File::Slurp;
use XML::LibXML;
use XML::LibXML::PrettyPrint;
use strict;

my $filename = 'netconf_generated.h';
open(my $header_fd, '>', $filename) or die "Could not open file '$filename' $!";

my @tags = split '\n', `./pretty_print_debug_file.pl debug_* 2> /dev/null | ./create_tags_from_pretty_prints.sh`;
my @namespaces = split '\n', `./generate_namespaces.sh`;

print $header_fd <<"EOF";
#ifndef NETCONF_GENERATED_H
#define NETCONF_GENERATED_H

enum xml_tag_type {
  XML_TAG_TYPE_UNKNOWN,
EOF
foreach my $tag (@tags) {
  my $formatted_tag = uc $tag;
  $formatted_tag =~ s/[-:\.]/_/g;
  print $header_fd "  XML_TAG_TYPE_" . $formatted_tag . ",\n";
}
print $header_fd <<'EOF';
};

enum xml_namespace {
  XML_NAMESPACE_UNKNOWN,
EOF

foreach my $namespace (@namespaces) {
  my $formatted_namespace = $namespace;
  $formatted_namespace =~ s/http[^\.]+\.([^\.]+)\.[^\/]+(\/.*)$/${1}${2}/;
  $formatted_namespace =~ s/urn:ietf:params:xml:/ietf:/;
  $formatted_namespace =~ s/[\/:]ns//;
  $formatted_namespace =~ s/[-:\.\/]/_/g;
  $formatted_namespace = uc $formatted_namespace;
  print $header_fd "  XML_NAMESPACE_" . $formatted_namespace . ",\n";
}
print $header_fd <<"EOF";
};

// nx_parse
struct nx_parse {
    char* name;
    enum xml_tag_type tag_type;
};

struct nx_xmlns_parse {
    char* name;
    enum xml_namespace namespace;
};

// nx_tree
enum nx_tree_enum {
  NX_TREE_ONT_MISSING,
  NX_TREE_DESCRIPTION,
  NX_TREE_PROBABLE_CAUSE,
  NX_TREE_DETAIL
};

struct nx_tree_parse {
    char* name;
    enum nx_tree_enum nx_tree;
/*
    enum cbor_actions action;
    // This string is empty or NULL if it is a direct match and there is no regex to do
    // Only the first element in each array will be used in that case, since there is
    // only one match (the entire string returned from libexpat)
    char *regex_match_string;
    char *keys[];
    enum cbor_types[];
*/
};

#endif //NETCONF_GENERATED_H
EOF

close($header_fd);

my $filename = 'nx_gperf.h';
open(my $nx_gperf_header_fd, '>', $filename) or die "Could not open file '$filename' $!";

print $nx_gperf_header_fd <<'EOF';
#ifndef NETCONF_GPERF_H
#define NETCONF_GPERF_H

static unsigned int
hash_nx (register const char *str, register size_t len);

struct nx_parse *
in_word_set_nx (register const char *str, register size_t len);

static unsigned int
hash_nx (register const char *str, register size_t len);

struct nx_xmlns_parse *
in_word_set_nx_xmlns (register const char *str, register size_t len);

#endif //NETCONF_GPERF_H
EOF

my $filename = 'nx_parse.gperf';
open(my $nx_parse_fd, '>', $filename) or die "Could not open file '$filename' $!";

print $nx_parse_fd <<'EOF';
%{
#include <stdlib.h>
#include <netconf_generated.h>
%}
%7bit
%define hash-function-name hash_nx
%define lookup-function-name in_word_set_nx
%define constants-prefix NX_
%define word-array-name wordlist_nx
%includes
struct nx_parse;
%%
EOF
foreach my $tag (@tags) {
  my $formatted_tag = uc $tag;
  $formatted_tag =~ s/[-:\.]/_/g;
  print $nx_parse_fd "$tag, XML_TAG_TYPE_" . $formatted_tag . "\n";
}
print $nx_parse_fd <<'EOF';
%%
EOF
close($nx_parse_fd);

my $filename = 'nx_xmlns_parse.gperf';
open(my $nx_xmlns_parse_fd, '>', $filename) or die "Could not open file '$filename' $!";

print $nx_xmlns_parse_fd <<'EOF';
%{
#include <stdlib.h>
#include <netconf_generated.h>
%}
%7bit
%define hash-function-name hash_nx
%define lookup-function-name in_word_set_nx_xmlns
%define constants-prefix NX_XMLNS_
%define word-array-name wordlist_nx_xmlns
%includes
struct nx_xmlns_parse;
%%
EOF
foreach my $namespace (@namespaces) {
  my $formatted_namespace = $namespace;
  $formatted_namespace =~ s/http[^\.]+\.([^\.]+)\.[^\/]+(\/.*)$/${1}${2}/;
  $formatted_namespace =~ s/urn:ietf:params:xml:/ietf:/;
  $formatted_namespace =~ s/[\/:]ns//;
  $formatted_namespace =~ s/[-:\.\/]/_/g;
  $formatted_namespace = uc $formatted_namespace;
  print $nx_xmlns_parse_fd "$namespace, XML_NAMESPACE_" . $formatted_namespace . "\n";
}
print $nx_xmlns_parse_fd <<'EOF';
%%
EOF
close($nx_parse_fd);
__END__

#foreach my $file (@ARGV) {
#  my $file_content = read_file($file);
#  my (@messages) = split "]]>]]>", $file_content;
#  foreach my $message (@messages) {
#    my $xml_parser = XML::LibXML->new;
#    my $doc = $xml_parser->parse_string($message);
#    my $pp = XML::LibXML::PrettyPrint->new(indent_string => "  ");
#    $pp->pretty_print($doc);
#    print $doc->toString;
#  }
#}


print <<"EOF";
enum nx_enum {
  NX_ONT_MISSING,
  NX_DESCRIPTION,
  NX_PROBABLE_CAUSE,
  NX_DETAIL
};

struct nx_parse {
    char* name;
    enum nx_enum nx;
};

static unsigned int
hash_nx (register const char *str, register size_t len);

struct nx_parse *
in_word_set_nx (register const char *str, register size_t len);
#endif //NX_PARSE_H
EOF

__END__

== nx_parse HEADER ==
#ifndef NX_PARSE_H
#define NX_PARSE_H

enum nx_enum {
  NX_ONT_MISSING,
  NX_DESCRIPTION,
  NX_PROBABLE_CAUSE,
  NX_DETAIL
};

struct nx_parse {
    char* name;
    enum nx_enum nx;
};

static unsigned int
hash_nx (register const char *str, register size_t len);

struct nx_parse *
in_word_set_nx (register const char *str, register size_t len);
#endif //NX_PARSE_H
================

== nx_parse GPerf Code ==
%{
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <nx_parse.h>
//enum nx_enum (struct nx_parse *nx_parser);
%}
%7bit
%define hash-function-name hash_nx
%define lookup-function-name in_word_set_nx
%define constants-prefix NX_
%define word-array-name wordlist_nx
%struct-type TODO: Figure out how this works
%includes
//%enum
struct nx_parse;
%%
ont-missing,              NX_ONT_MISSING
description,              NX_DESCRIPTION
probable-cause,           NX_PROBABLE_CAUSE
detail,                   NX_DETAIL
%%
// Other code or declarations
===========

== nx_attr_parse HEADER ==
#ifndef NX_XMLNS_PARSE_H
#define NS_XMLNS_PARSE_H

enum nx_xmlns_enum {
  NX_XMLNS_ONT_MISSING,
  NX_XMLNS_DESCRIPTION,
  NX_XMLNS_PROBABLE_CAUSE,
  NX_XMLNS_DETAIL
};

struct nx_xmlns_parse {
    char* name;
    enum nx_xmlns_enum nx_xmlns;
};

static unsigned int
hash_nx_xmlns (register const char *str, register size_t len);

struct nx_xmlns_parse *
in_word_set_nx_xmlns (register const char *str, register size_t len);
#endif //NX_XMLNS_PARSE_H
================

== nx_attr_parse GPerf Code ==
%{
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <nx_xmlns_parse.h>
//enum nx_xmlns_enum (struct nx_xmlns_parse *nx_parser);
%}
%7bit
%define hash-function-name hash_nx_xmlns
%define lookup-function-name in_word_set_nx_xmlns
%define constants-prefix NX_XMLNS_
%define word-array-name wordlist_nx_xmlns
%struct-type TODO: Figure out how this works
%includes
//%enum
struct nx_xmlns_parse;
%%
http://www.calix.com/ns/exa/gpon-interface-base,                79,NULL,calix_doc_tag_end,NULL
%%
// Other code or declarations
===========

== nx_tree_parse HEADER ==
#ifndef NX_TREE_PARSE_H
#define NS_TREE_PARSE_H

enum nx_tree_enum {
  NX_TREE_ONT_MISSING,
  NX_TREE_DESCRIPTION,
  NX_TREE_PROBABLE_CAUSE,
  NX_TREE_DETAIL
};

union Strings {
    char *string;
    char *strings[];
};

enum cbor_actions{
  CBOR_NEW_ROOT_MAP,
  CBOR_NEW_MAP,
  CBOR_NEW_ARRAY,
  CBOR_NEW_INT,
  CBOR_NEW_STRING,
  CBOR_NEW_BOOLEAN,
  CBOR_NEW_PAIR
};

enum cbor_types {
  CBOR_TYPE_INT,
  CBOR_TYPE_STRING,
  CBOR_TYPE_BOOLEAN,
  CBOR_TYPE_ARRAY,
  CBOR_TYPE_MAP
};

struct nx_tree_parse {
    char* name;
    enum nx_tree_enum nx_tree;
    enum cbor_actions action;
    // This string is empty or NULL if it is a direct match and there is no regex to do
    // Only the first element in each array will be used in that case, since there is
    // only one match (the entire string returned from libexpat)
    char *regex_match_string;
    char *keys[];
    enum cbor_types[];
};

static unsigned int
hash_nx_tree (register const char *str, register size_t len);

struct nx_tree_parse *
in_word_set_nx_tree (register const char *str, register size_t len);
#endif //NX_TREE_PARSE_H
================

== nx_tree_parse GPerf Code ==
%{
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <nx_tree_parse.h>
//enum nx_tree_enum (struct nx_tree_parse *nx_parser);
%}
%compare-lengths
//%7bit
%define hash-function-name hash_nx_tree
%define lookup-function-name in_word_set_nx_tree
%define constants-prefix NX_TREE_
%define word-array-name wordlist_nx_tree
%struct-type TODO: Figure out how this works
%includes
//%enum
struct nx_tree_parse;
%%
#Top level
#\000\000\000\000\000\000\000\000
# Notification
\000\000\000\001\000\000\000\000
# RPC Reply
\000\000\000\002
#Store ONT_FULL_DETAIL
\000\000\000\003
\000\000\000\003\000\000\000\001, NX_TREE_Z,CBOR_NEW_ROOT_MAP,"",{"ont_full_detail"},{CBOR_NEW_ROOT_MAP}
\000\000\000\003\000\000\000\002, NX_TREE_X,CBOR_NEW_PAIR,"",{"ont_id"},{CBOR_TYPE_INT}
\000\000\000\003\000\000\000\003, NX_TREE_Y,CBOR_NEW_PAIR,"^([A-Z][a-z]+) ([A-Z][a-z]+)$",["ip_adress", "mac_address"],[CBOR_TYPE_STRING, CBOR_TYPE_STRING]
%%
// Other code or declarations
===========

{
  "message_type": "notification",
  "event_time": "2025-02-21T16:37:15-06:00",
  "actual_event_time": "1234567890",
  "notification_type": "dhcp-lease-establishment",
  "shelf": 1,
  "slot": 1,
  "ont_id": 108,
  "port": "g1",
  "vlan_s_tag": 2026,
  "vlan_c_tag": 3,
  "ip_address": "209.95.109.207",
  "mac_address": "48:77:46:d3:a2:69"
}

<notification xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0">
  <eventTime>2025-02-21T16:37:15-06:00</eventTime>
  <dhcp-lease-establishment xmlns="http://www.calix.com/ns/exa/layer2-service-protocols">
    <description>Lease establishment</description>
    <probable-cause>Lease Established</probable-cause>
    <details>Client initiated lease establishment. ip-address :209.95.109.207,mac-address :48:77:46:d3:a2:69</details>
    <device-sequence-number>2766</device-sequence-number>
    <instance-id>9.4682</instance-id>
    <id>2303</id>
    <name>dhcp-lease-establishment</name>
    <alarm>false</alarm>
    <category>GENERAL</category>
    <address>/interfaces/interface[name='108/G1']/vlan[vlan-id='2026']/c-vlan[c-vlan-id='3']</address>
    <shelf-id>1</shelf-id>
    <slot-id>1</slot-id>
    <ont-id>108</ont-id>
    <port>G1</port>
    <vlan-id>2026</vlan-id>
    <c-vlan-id>3</c-vlan-id>
  </dhcp-lease-establishment>
</notification>

{
  "message_type": "ont_detail",
  "rpc_message_id": 487,
  "ont_id": 23621,
  "fsan": "CXNK7DD02A",
  #<mfg-serial-number>422004004992</mfg-serial-number>
  "serial_number": "422004004992",


  "shelf": 1,
  "slot": 1,
  "ont_id": 108,
  "port": "g1",
  "vlan_s_tag": 2026,
  "vlan_c_tag": 3,
  "ip_address": "209.95.109.207",
  "mac_address": "48:77:46:d3:a2:69"
}


<rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="487">
  <data>
    <status xmlns="http://www.calix.com/ns/exa/base">
      <system>
        <ont xmlns="http://www.calix.com/ns/exa/gpon-interface-base">
          <ont-id>108</ont-id>
          <status>
            <vendor>CXNK</vendor>
            <serial-number>7DD02A</serial-number>
            <upgrade-class>none</upgrade-class>
            <oper-state>present</oper-state>
            <linked-pon>1/1/gp1</linked-pon>
            <model>844G-1</model>
            <clei>BVMCH00ARE</clei>
            <curr-version>12.2.13.0.49</curr-version>
            <alt-version>12.2.11.0.129</alt-version>
            <voip-config-version>0</voip-config-version>
            <rg-config-version>0</rg-config-version>
            <curr-committed>true</curr-committed>
            <onu-mac-addr>48:77:46:d3:a2:69</onu-mac-addr>
            <mta-mac-addr>48:77:46:d3:a2:6a</mta-mac-addr>
            <mfg-serial-number>422004004992</mfg-serial-number>
            <product-code>P0</product-code>
          </status>
          <linkage>
            <status>Confirmed</status>
            <linked-by>Serial-Number</linked-by>
          </linkage>
          <detail>
            <range-length>23795</range-length>
            <up-time>5522909</up-time>
            <opt-signal-level>-18.000</opt-signal-level>
            <tx-opt-level>2.000</tx-opt-level>
            <ne-opt-signal-level>-23.200</ne-opt-signal-level>
            <us-ber-monitor>bip</us-ber-monitor>
            <ds-ber-monitor>bip</ds-ber-monitor>
            <us-sdber-rate>1.00E-14</us-sdber-rate>
            <ds-sdber-rate>1.00E-14</ds-sdber-rate>
          </detail>
          <counters>
            <ont-counters>
              <upstream-bip-errors>1</upstream-bip-errors>
              <upstream-missed-bursts>0</upstream-missed-bursts>
              <upstream-gem-hec-errors>0</upstream-gem-hec-errors>
              <upstream-bip8-err-sec>1</upstream-bip8-err-sec>
              <upstream-bip8-severely-err-sec>0</upstream-bip8-severely-err-sec>
              <upstream-bip8-unavail-sec>0</upstream-bip8-unavail-sec>
              <upstream-missed-bursts-err-sec>0</upstream-missed-bursts-err-sec>
              <upstream-fec-corrected-bytes>0</upstream-fec-corrected-bytes>
              <upstream-fec-corrected-code-words>0</upstream-fec-corrected-code-words>
              <upstream-fec-uncorrected-code-words>0</upstream-fec-uncorrected-code-words>
              <upstream-fec-total-code-words>0</upstream-fec-total-code-words>
              <upstream-fec-sec>0</upstream-fec-sec>
              <downstream-bip-errors>0</downstream-bip-errors>
              <downstream-bip8-err-sec>0</downstream-bip8-err-sec>
              <downstream-bip8-severely-err-sec>0</downstream-bip8-severely-err-sec>
              <downstream-bip8-unavail-sec>0</downstream-bip8-unavail-sec>
              <downstream-fec-corrected-bytes>0</downstream-fec-corrected-bytes>
              <downstream-fec-corrected-code-words>0</downstream-fec-corrected-code-words>
              <downstream-fec-uncorrected-code-words>0</downstream-fec-uncorrected-code-words>
              <downstream-fec-total-code-words>786226608000</downstream-fec-total-code-words>
              <downstream-fec-sec>0</downstream-fec-sec>
              <doze-time>0</doze-time>
              <cyclic-sleep-time>0</cyclic-sleep-time>
              <watchful-sleep-time>0</watchful-sleep-time>
              <energy-consumed>0</energy-consumed>
              <cpu-percent-utilization>0</cpu-percent-utilization>
              <ram-available-amount>0</ram-available-amount>
              <ram-utilization>0</ram-utilization>
              <flash-available-amount>0</flash-available-amount>
              <flash-utilization>0</flash-utilization>
              <software-errors>0</software-errors>
              <errors-in-operations>0</errors-in-operations>
            </ont-counters>
          </counters>
          <pon-us-cos-status>
            <pon-cos>
              <name>cos-1</name>
              <policy>sp</policy>
              <queue>
                <id>1</id>
                <reference-count>1</reference-count>
                <max-size>256</max-size>
                <queue-depth>256</queue-depth>
                <weight>1</weight>
                <green-minimum-drop-threshold>512</green-minimum-drop-threshold>
                <green-maximum-drop-threshold>512</green-maximum-drop-threshold>
                <green-packet-drop-probability>256 (100.00%)</green-packet-drop-probability>
                <yellow-minimum-drop-threshold>512</yellow-minimum-drop-threshold>
                <yellow-maximum-drop-threshold>512</yellow-maximum-drop-threshold>
                <yellow-packet-drop-probability>256 (100.00%)</yellow-packet-drop-probability>
                <queue-drop-averaging-coefficient>2^(-9)</queue-drop-averaging-coefficient>
              </queue>
            </pon-cos>
          </pon-us-cos-status>
          <utilization-counters>
            <service-stats-group>cco-108</service-stats-group>
            <pon>1/1/gp1</pon>
            <upstream-octets>2287861887</upstream-octets>
            <downstream-octets>79841743698</downstream-octets>
            <upstream-pon-rate>1145 Mbps</upstream-pon-rate>
            <downstream-pon-rate>2216 Mbps</downstream-pon-rate>
            <last-upstream-percentage>0.012</last-upstream-percentage>
            <last-downstream-percentage>0.058</last-downstream-percentage>
          </utilization-counters>
          <qos>
            <pon-qos>
              <bandwidth>
                <index>0</index>
                <vlan>2026</vlan>
                <c-vlan>3</c-vlan>
                <port>108/G1</port>
                <direction>ingress</direction>
                <type>meter</type>
                <pon-cos>cos-1</pon-cos>
                <dba-priority>BE-1</dba-priority>
                <cir>0</cir>
                <eir>100000</eir>
                <pon-cos-air>0</pon-cos-air>
                <pon-cos-eir>100000</pon-cos-eir>
              </bandwidth>
            </pon-qos>
          </qos>
        </ont>

{
  "message_type": "ont_us_sdber_active",
  "calix_system_ip": "192.168.35.14",
  "rpc_message_id": 487,
  "ont_id": 33318,
  "fsan": "CXNK7DD02A",
  "shelf": 1,
  "slot": 1,
  "port": "gp4"
}

struct ont_us_sdber_table {
  uint32_t ont_id,
  uint8_t shelf,
  uint8_t slot,
  char *port,
  char *vendor_id,
  char *serial_number,
  bool in_alarm,
};

  <ont-us-sdber xmlns="http://www.calix.com/ns/exa/gpon-interface-base">
    <description>ONT Upstream SDBER rate exceeded</description>
    <probable-cause>Upstream Signal Degrade Bit Error Rate (SDBER) exceeded. </probable-cause>
    <details>SerialNo=12C6BB6</details>
    <device-sequence-number>6801</device-sequence-number>
    <instance-id>10.6801</instance-id>
    <id>5033</id>
    <name>ont-us-sdber</name>
    <perceived-severity>MINOR</perceived-severity>
    <alarm>true</alarm>
    <service-impacting>true</service-impacting>
    <service-affecting>false</service-affecting>
    <alarm-type>QOS</alarm-type>
    <category>PON</category>
    <hw-failure-event>false</hw-failure-event>
    <repair-action>Check optical power and attenuation. Fiber cleaning maybe needed. </repair-action>
    <address>/config/system/ont[ont-id='33318']</address>
    <port>gp4</port>
    <ont-id>33318</ont-id>
    <phys-pon-shelf>1</phys-pon-shelf>
    <phys-pon-slot>1</phys-pon-slot>
    <phys-pon-port>gp4</phys-pon-port>
    <serial-number>12C6BB6</serial-number>
    <equipment-type>ONT</equipment-type>
    <vendor-id>CXNK</vendor-id>
  </ont-us-sdber>

<notification xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0">
  <eventTime>2025-02-21T12:31:49-06:00</eventTime>
  <ont-us-sdber xmlns="http://www.calix.com/ns/exa/gpon-interface-base">
    <description>ONT Upstream SDBER rate exceeded</description>
    <probable-cause>Upstream Signal Degrade Bit Error Rate (SDBER) exceeded. </probable-cause>
    <details>SerialNo=12C6BB6</details>
    <device-sequence-number>6798</device-sequence-number>
    <instance-id>10.6796</instance-id>
    <id>5033</id>
    <name>ont-us-sdber</name>
    <perceived-severity>CLEAR</perceived-severity>
    <prev-severity>MINOR</prev-severity>
    <alarm>true</alarm>
    <service-impacting>true</service-impacting>
    <service-affecting>false</service-affecting>
    <alarm-type>QOS</alarm-type>
    <category>PON</category>
    <hw-failure-event>false</hw-failure-event>
    <repair-action>Check optical power and attenuation. Fiber cleaning maybe needed. </repair-action>
    <address>/config/system/ont[ont-id='33318']</address>
    <port>gp4</port>
    <ont-id>33318</ont-id>
    <phys-pon-shelf>1</phys-pon-shelf>
    <phys-pon-slot>1</phys-pon-slot>
    <phys-pon-port>gp4</phys-pon-port>
    <serial-number>12C6BB6</serial-number>
    <equipment-type>ONT</equipment-type>
    <vendor-id>CXNK</vendor-id>
  </ont-us-sdber>
</notification>

{
  "message_type": "user_login",
  "remote_ip": "192.168.32.40",
  "user_name": "sysadmin",
  "connection_type": "cli"
}

<notification xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0">
  <eventTime>2025-02-21T12:42:03-06:00</eventTime>
  <user-login xmlns="http://www.calix.com/ns/exa/base">
    <description>User login from CLI/Netconf/EWI</description>
    <probable-cause>User login from CLI/Netconf/EWI</probable-cause>
    <details>This event is generated when a user logs into CLI/Netconf/EWI</details>
    <device-sequence-number>2738</device-sequence-number>
    <instance-id>10.6799</instance-id>
    <id>101</id>
    <name>user-login</name>
    <alarm>false</alarm>
    <category>SECURITY</category>
    <address>/config/system/aaa/user[name='sysadmin']</address>
    <session-id>1144</session-id>
    <session-login>sysadmin</session-login>
    <session-manager>cli</session-manager>
    <session-ip>192.168.32.40</session-ip>
    <user-name>sysadmin</user-name>
  </user-login>
</notification>
