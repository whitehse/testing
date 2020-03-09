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

#chmod 700 private
#touch index.txt
#echo 1000 > serial
#openssl genrsa -out private/ca.key.pem 4096
#chmod 400 private/ca.key.pem
#openssl req -config "$CUR_DIR"/root-config.txt \
#      -key private/ca.key.pem \
#      -new -x509 -days 7300 -sha256 -extensions v3_ca \
#      -out certs/ca.cert.pem \
#	  -subj "$SUBJECT"
#chmod 444 certs/ca.cert.pem
