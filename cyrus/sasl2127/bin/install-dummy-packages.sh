#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

export HOST="$1"

/usr/bin/scp -F conf/ssh_config dummy-packages/*deb root@ldap23.example.com:

ssh -F conf/ssh_config root@firewall 'bash -s' << EOF
EOF
