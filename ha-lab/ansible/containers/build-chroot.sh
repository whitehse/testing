#!/bin/bash

if [ -z "$1" ]; then
  echo "multistrap config file missing"
  echo "Usage: build-chroot.sh <multistrap config file> <build directory>"
  exit 1
fi

if [ -z "$2" ]; then
  echo "build directory missing missing"
  echo "Usage: build-chroot.sh <multistrap config file> <build directory>"
  exit 1
fi

MULTISTRAP_CONF="$1"
BUILD_DIR="$2"

sudo multistrap -d "$BUILD_DIR" -f "$MULTISTRAP_CONF"
sudo chroot "$BUILD_DIR" /configscript.sh
sudo cp files/init.d-frr "$BUILD_DIR/etc/init.d/frr"
sudo chmod 755 "$BUILD_DIR/etc/init.d/frr"
sudo cp files/daemons* "$BUILD_DIR/etc/frr/"
sudo cp files/interfaces "$BUILD_DIR/etc/network/"
sudo cp files/zebra.conf "$BUILD_DIR/etc/frr/zebra.conf"
sudo chroot build adduser frr frrvty
sudo cp -a build build.orig

# Left to do. Create /etc/frr/bgpd.conf, and the files
# underneath /etc/network/interfaces.d/
