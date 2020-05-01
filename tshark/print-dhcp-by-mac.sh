#!/bin/bash

MAC="$1"

tshark -n -f "port 67 or port 68" -Y "dhcp.hw.mac_addr==${MAC}" \
  -O bootp
