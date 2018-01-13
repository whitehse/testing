#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

for foo in openssl libssl1.1 libssl-dev libssl-doc libssl1.0.2 libssl1.0-dev
do
  bin/install-dummy-package.sh $1 $foo
done
