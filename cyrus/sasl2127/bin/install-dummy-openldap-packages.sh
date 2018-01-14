#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

for foo in slapd slapd-smbk5pwd ldap-utils libldap-2.4-2 libldap-common libldap2-dev
do
  bin/install-dummy-package.sh $1 $foo
done
