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
apt-get -y build-dep libssl1.0.2
cd /usr/src/
wget --no-check-certificate https://www.openssl.org/source/openssl-1.0.2n.tar.gz
tar -xvzf openssl*gz
cd openssl-1.0.2n
./Configure shared --prefix=/usr --openssldir=/usr/lib/ssl --libdir=lib no-idea no-mdc2 no-rc5 no-zlib no-ssl3 enable-unit-test no-ssl3-method enable-rfc3779 enable-cms enable-ec_nistp_64_gcc_128 linux-x86_64 && make depend && make && make install
EOF
