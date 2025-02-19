#!/usr/bin/perl

use warnings;
use strict;
use Mojo::IOLoop;
use Mojo::Reactor::EV;
use IPC::Open3;
use FileHandle;
#use Proc::Background;
use XML::Hash::XS;
use Data::Dumper;
use XML::SAX::Expat;
use XML::SAX::Expat::Incremental;
use MySAXHandler;
 
BEGIN {
    $ENV{MOJO_REACTOR} = 'Mojo::Reactor::EV';
}

# Connection states
use constant {
    READY_TO_SEND_HELLO            => 0,
    READY_TO_RECEIVE_HELLO         => 1,
    READY_TO_SUBSCRIBE             => 2,
    READY_TO_RECEIVE_SUBSCRIBE_ACK => 3,
    READY_TO_RECEIVE_NOTIFICATIONS => 4,
};

my $hello_message = <<'END_MESSAGE';
<?xml version="1.0" encoding="UTF-8"?>
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
<capabilities>
<capability>urn:ietf:params:netconf:base:1.0</capability>
</capabilities>
</hello>
]]>]]>
END_MESSAGE
chomp $hello_message;

my $get_config = <<'END_MESSAGE';
<?xml version="1.0" encoding="UTF-8"?>
<rpc message-id="101" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <get-config>
    <source>
      <running/>
    </source>
  </get-config>
</rpc>
]]>]]>
END_MESSAGE
chomp $get_config;

my $get_interfaces = <<'END_MESSAGE';
<?xml version="1.0" encoding="UTF-8"?>
<rpc message-id="102" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <get>interface</get>
</rpc>
]]>]]>
END_MESSAGE
chomp $get_interfaces;

my $subscribe = <<'END_MESSAGE';
<?xml version="1.0" encoding="UTF-8"?>
<netconf:rpc message-id="101" xmlns:netconf="urn:ietf:params:xml:ns:netconf:base:1.0">
    <create-subscription xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0">
        <stream>exa-events</stream>
    </create-subscription>
</netconf:rpc>
]]>]]>
END_MESSAGE

my $get_ont_status = <<'END_MESSAGE';
<rpc xmlns="urn:ietf:params:xml:ns:netconf:base:0.0" message-id="487">
  <get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <filter type="xpath" select="/status/system/ont"/>
  </get>
</rpc>
]]>]]>
END_MESSAGE

my %sockets;

my $reactor = Mojo::Reactor::EV->new;

my @e7s = (
#  "192.168.21.12",
#  "192.168.21.3",
#  "192.168.21.4",
#  "192.168.21.5",
#  "192.168.21.6",
#  "192.168.21.7",
#  "192.168.21.8",
#  "192.168.21.9",
#  "192.168.21.11",
#  "192.168.21.12",
#  "192.168.21.13",
#  "192.168.21.14",
#  "192.168.21.10",
#  "192.168.21.15",
#  "192.168.21.16",
#  "192.168.22.2",
#  "192.168.22.3",
#  "192.168.22.4",
#  "192.168.22.5",
#  "192.168.22.6",
#  "192.168.22.7",
#  "192.168.22.8",
#  "192.168.22.9",
#  "192.168.35.11",
  "192.168.35.12",
#  "192.168.35.13",
#  "192.168.35.14",
#  "192.168.35.15",
#  "192.168.21.17",
#  #"192.168.34.11",
#  #"192.168.34.12",
#  #"192.168.34.13",
#  #"192.168.34.14",
#  "192.168.22.15",
#  #"192.168.33.11",
#  #"192.168.33.12",
#  #"192.168.33.13",
#  #"192.168.33.14",
#  #"192.168.36.11",
#  #"192.168.36.12",
#  #"192.168.36.13",
#  "192.168.43.10",
#  "192.168.43.11",
#  "192.168.43.12",
#  "192.168.43.13",
#  "192.168.43.14",
#  "192.168.42.11",
#  "192.168.42.12",
#  "192.168.42.13",
#  "192.168.42.14",
#  "192.168.42.15",
#  "192.168.42.16",
#  "192.168.42.17",
#  "192.168.42.18",
#  "192.168.47.11",
#  "192.168.47.12",
#  "192.168.47.13",
#  "192.168.47.14",
#  "192.168.47.15",
#  "192.168.51.11",
#  "192.168.51.12",
#  "192.168.51.13",
#  "192.168.51.14",
#  "192.168.51.15",
#  "192.168.51.16",
#  "192.168.44.11",
#  "192.168.44.12",
#  "192.168.44.13",
#  "192.168.44.14",
#  "192.168.44.15",
#  "192.168.49.11",
#  "192.168.49.12",
#  "192.168.46.11",
#  "192.168.46.12",
#  "192.168.46.13",
#  "192.168.46.14",
#  "192.168.46.15",
#  "192.168.46.16",
#  "192.168.46.17",
#  "192.168.46.18",
#  "192.168.39.14",
#  "192.168.44.16",
#  "192.168.51.17",
#  "192.168.51.18",
#  "192.168.45.11",
#  "192.168.45.12",
#  "192.168.45.13",
#  "192.168.45.14",
#  "192.168.48.11",
#  "192.168.48.12",
#  "192.168.48.13",
#  "192.168.48.14",
#  "192.168.48.15",
#  "192.168.48.16",
);

foreach my $e7 (@e7s) {
 
    print ("Connecting to $e7\n");
    #my $ssh_in;
    #my $ssh_out;
    #my $ssh_err;
    my $ssh_in = FileHandle->new;
    my $ssh_out = FileHandle->new;
    my $ssh_err = FileHandle->new;

    my $pid = open3($ssh_in, $ssh_out, $ssh_err,
     'sshpass -p sysadmin ssh -p 830 -s sysadmin@' . $e7 . " netconf");

    $sockets{$e7}{in} = $ssh_in;
    $sockets{$e7}{in_readable} = 0;
    $sockets{$e7}{out} = $ssh_out;
    $sockets{$e7}{out_readable} = 0;
    $sockets{$e7}{out_buffer} = '';
    $sockets{$e7}{err} = $ssh_err;
    $sockets{$e7}{err_readable} = 0;
    $sockets{$e7}{pid} = $pid;
    #$sockets{$e7}{proc} = $proc;
    #$sockets{$e7}{state} = READY_TO_SEND_HELLO;
    my $handle_in = IO::Handle->new_from_fd($sockets{$e7}{out}, 'r');
    $sockets{$e7}{handle_in} = $handle_in;
    my $handle_out = IO::Handle->new_from_fd($sockets{$e7}{in}, 'r');
    $sockets{$e7}{handle_out} = $handle_out;
    my $conv = XML::Hash::XS->new(utf8 => 0, encoding => 'utf-8');
    my $parser = XML::SAX::Expat::Incremental->new( Handler => MySAXHandler->new );
    $parser->parse_start;
    $sockets{$e7}{parser} = $parser;

    $reactor->io($handle_in => sub {
        my ($reactor, $writable) = @_;
        local $/ = "]]>]]>";
        my $block_size = 0;
        my $buffer;
        my $n = sysread($handle_in, $buffer, 64*1024);
        #print "bytes read = $n from $e7\n";
        #print "buffer is =========\n" . $buffer . "\n=========\n";
        $sockets{$e7}{out_buffer} = $sockets{$e7}{out_buffer} . $buffer;
        if (! $buffer =~ /]]>]]>$/) {
            print "buffer is =========\n" . $buffer . "\n=========\n";
            $parser->parse_more($buffer);
        } elsif ($sockets{$e7}{out_buffer} =~ /]]>]]>$/) {  
            chomp $buffer;
            print "buffer is =========\n" . $buffer . "\n=========\n";
            $parser->parse_more($buffer);
            $parser = XML::SAX::Expat::Incremental->new( Handler => MySAXHandler->new );
            $parser->parse_start;
            my @responses = split /]]>]]>/, $sockets{$e7}{out_buffer};
            foreach my $response (@responses) {
                chomp $response;
                my $xml_hash = $conv->xml2hash($response, encoding => 'utf-8');
                #print Dumper($xml_hash);
                if ($sockets{$e7}{state} == READY_TO_RECEIVE_SUBSCRIBE_ACK) {
                    print("Sending NETCONF get_ont_status, for $e7\n");
                    print ($get_ont_status);
                    print { $sockets{$e7}{in} } $get_ont_status;
                    $sockets{$e7}{state} = READY_TO_RECEIVE_NOTIFICATIONS;
                } elsif ($sockets{$e7}{state} == READY_TO_RECEIVE_HELLO) {
                    print("Sending NETCONF subscribe, for $e7\n");
                    print ($subscribe);
                    print { $sockets{$e7}{in} } $subscribe;
                    $sockets{$e7}{state} = READY_TO_RECEIVE_SUBSCRIBE_ACK;
                }
            }
            $sockets{$e7}{out_buffer} = '';
        }
    })->watch($handle_in, 1, 0);

    print("Sending NETCONF client <hello>, for $e7\n");
    print $hello_message;
    print { $sockets{$e7}{in} } $hello_message;
    #print $ssh_in $hello_message;
    my $bytes_written;
    $sockets{$e7}{state} = READY_TO_RECEIVE_HELLO;
}

$reactor->start unless $reactor->is_running;
