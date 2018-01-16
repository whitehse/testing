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
apt-get -y build-dep openafs
cd /usr/src
wget --no-check-certificate https://kerberos.org/dist/krb5/1.16/krb5-1.16.tar.gz
tar -xvzf openafs-1.6.22.1-src.tar.gz
cd openafs-1.6.22.1
./configure \
with-afs-sysname=$(HOSTNAME) \

make && make install
ldconfig
apt-get -y openafs-modules-dkms
EOF
