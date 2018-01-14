#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

for foo in libgnutls28-dev libgnutls30 gnutls-bin gnutls-doc libgnutlsxx28 libgnutls-openssl27 libgnutls-dane0
do
  bin/install-dummy-package.sh $1 $foo
done
