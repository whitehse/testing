#!/bin/bash

HOST_PORT="$1"

openssl s_client -showcerts -connect $HOST_PORT </dev/null 2>/dev/null|openssl x509 -text -noout
