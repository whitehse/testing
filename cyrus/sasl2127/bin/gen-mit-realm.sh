#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

export HOST="$1"

ssh -F conf/ssh_config $HOST 'bash -l -s' << EOF
echo "
" | kdb5_util create -r EXAMPLE.COM -s
echo "
addprinc dwhite@EXAMPLE.COM
passw0rd
passw0rd
addprinc -randkey host/ldap.example.com
addprinc -randkey ldap/ldap.example.com
ktadd -k ldap.keytab host/ldap.example.com
ktadd -k slapd.keytab ldap/ldap.example.com
addprinc -randkey host/imap.example.com
addprinc -randkey imap/imap.example.com
addprinc -randkey smtp/imap.example.com
ktadd -k imap.keytab host/imap.example.com
ktadd -k imapd.keytab imap/imap.example.com
ktadd -k smtpd.keytab smtp/imap.example.com
q" | kadmin.local
EOF
