#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A build directory must be specified.
EOF
  exit 1
fi

export BUILDDIR="$1"

if [ ! -d "$BUILDDIR" ]; then
  cat - << EOF
The specified build directory does not exist.
EOF
  exit 1
fi

if [ -z "$2" ]; them
  cat - << EOF
A hostname must be specified.
EOF
  exit 1
fi

HOSTNAME="$2"

if [ -z "$3" ]; then
  cat - << EOF
A virtual switch name must be specified - example.com,example.org.
EOF
  exit 1
fi

SWITCH="$3"

sudo linux ubd0="$BUILDDIR/${HOSTNAME}.img" mem=256M con=pty con0=fd:0,fd:1 eth0=vde,"$BUILDDIR/${SWITCH}" umid=${HOSTNAME}
