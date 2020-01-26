#!/bin/bash

INT=$1

tshark -n -i $INT -f "port 500" -Y 'bgp.type == 2' \
  -T fields \
  -e frame.time \
  -e bgp.nlri_prefix \
  -e bgp.prefix_length \
  -e bgp.update.path_attribute.community_as \
  -e bgp.update.path_attribute.community_value
