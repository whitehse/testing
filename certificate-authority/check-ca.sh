#!/bin/bash

if [ -z "$1" ]; then
  cat - << EOF
Must provide the root direcotry of the ca:
 ./create-root-cert.sh <ca_root>

For example:
 ./create-root-cert.sh ca/
EOF
  exit 1
fi

export DIR="$1"

if [ ! -d "$DIR" ]; then
  echo "$DIR must exist"
  exit 2
fi

if [ ! -d "$DIR/certs" ]; then
  echo "$DIR/certs does not exist. Exiting"
  exit 3
fi

CUR_DIR=`pwd`
cd "$DIR"

#cat intermediate/certs/intermediate.cert.pem \
#      certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem

#openssl verify -CAfile intermediate/certs/ca-chain.cert.pem \
#  intermediate/certs/\${foo}.cert.pem

#openssl verify -CAfile intermediate/certs/ca-chain.cert.pem \
#      intermediate/certs/dwhite.example.com.cert.pem
