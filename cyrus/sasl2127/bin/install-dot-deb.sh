#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

if [ -z "$2" ]; then
  cat - << EOF
A .deb file must be specified.
EOF
  exit 1
fi

export HOST="$1"
export DEB="$2"

FILENAME=$(basename "$DEB")

echo "scp -F conf/ssh_config $DEB root@${HOST}:"
scp -F conf/ssh_config $DEB root@${HOST}:

ssh -F conf/ssh_config root@${HOST} 'bash -l -s' << EOF
dpkg -i $FILENAME
EOF
