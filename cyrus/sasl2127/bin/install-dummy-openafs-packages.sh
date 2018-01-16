#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

for foo in openafs-client openafs-fuse openafs-kpasswd openafs-fileserver openafs-dbserver openafs-doc openafs-krb5 libkopenafs1 libafsauthent1 libafsrpc1 libopenafs-dev libpam-openafs-kaserver
do
  bin/install-dummy-package.sh $1 $foo
done
