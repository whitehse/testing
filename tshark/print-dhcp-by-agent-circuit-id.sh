#!/bin/bash

AGENT_ID="$1"

ENC_AGENT_ID=`echo -n "$AGENT_ID" | od -An -t x1 --endian=big | sed 's/ \+/:/g' | sed 's/^://'`

tshark -n -f "port 67 or port 68" \
  -Y "dhcp.option.agent_information_option.agent_circuit_id==${ENC_AGENT_ID}" \
  -O bootp
