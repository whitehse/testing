#!/usr/bin/env perl

#use Text::CSV qw( csv );
use Data::Dumper;
use File::Slurp;
use XML::LibXML;
use XML::LibXML::PrettyPrint;
use strict;

foreach my $file (@ARGV) {
  my $file_content = read_file($file);
  my (@messages) = split "]]>]]>", $file_content;
  foreach my $message (@messages) {
    my $xml_parser = XML::LibXML->new;
    my $doc = $xml_parser->parse_string($message);
    my $pp = XML::LibXML::PrettyPrint->new(indent_string => "  ");
    $pp->pretty_print($doc);
    print $doc->toString;
  }
}
