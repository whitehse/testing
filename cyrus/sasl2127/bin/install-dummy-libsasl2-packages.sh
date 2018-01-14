#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

for foo in sasl2-bin cyrus-sasl2-doc libsasl2-2 libsasl2-modules libsasl2-modules-db libsasl2-modules-ldap libsasl2-modules-otp libsasl2-modules-sql libsasl2-modules-gssapi-mit libsasl2-dev libsasl2-modules-gssapi-heimdal
do
  bin/install-dummy-package.sh $1 $foo
done
