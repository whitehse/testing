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

sudo linux ubd0="$BUILDDIR/firewall.img" mem=256M con=pty con0=fd:0,fd:1 eth0=tuntap,tap27,, eth1=vde,"$BUILDDIR/example.com" eth2=vde,"$BUILDDIR/example.org" umid=firewall
