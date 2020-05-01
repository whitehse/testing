#!/bin/bash

tshark -l -n -f "port 67 or port 68" -T fields -E separator=, \
  -e frame.time_relative \
  -e dhcp.hw.mac_addr \
  -e dhcp.ip.your \
  -e dhcp.ip.relay \
  -e dhcp.option.dhcp \
  -e dhcp.option.requested_ip_address \
  -e dhcp.option.agent_information_option.agent_circuit_id | \
\
  awk -F',' '{print "\033[39m"$1" \033[33m"$2" \033[34m"$3"\033[39m"}' # format string here
