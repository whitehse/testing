#!/bin/bash

if [ -z "$2" ]; then
  cat - << EOF
Must provide two options:
 ./create-root-cert.sh <output_directory> <subject>

For example:
 ./create-root-cert.sh ca/ "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Root CA"
EOF
  exit 1
fi

export DIR="$1"
export SUBJECT="$2"

if [ ! -d "$DIR" ]; then
  echo "$DIR must exist"
  exit 2
fi

if [ -d "$DIR/certs" ]; then
  echo "$DIR/certs exists, indicating an existing root CA. Exiting"
  exit 3
fi

root_cnf=`mktemp`
cat root-config.txt | sed "s/DIR/${DIR}/" > $root_cnf
echo "finished sed command"

mkdir -p "$DIR"/certs
mkdir -p "$DIR"/crl
mkdir -p "$DIR"/newcerts
mkdir -p "$DIR"/private

chmod 700 "$DIR"/private
touch "$DIR"/index.txt
echo 1000 > "$DIR"/serial
openssl genrsa -out "$DIR"/private/ca.key.pem 4096
chmod 400 "$DIR"/private/ca.key.pem
openssl req -config $root_cnf \
      -key "$DIR"/private/ca.key.pem \
      -new -x509 -days 7300 -sha256 -extensions v3_ca \
      -out "$DIR"/certs/ca.cert.pem \
	  -subj "$SUBJECT"
chmod 444 "$DIR"/certs/ca.cert.pem

rm $root_cnf
