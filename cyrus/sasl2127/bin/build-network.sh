#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A build directory must be specified.
EOF
  exit 1
fi

BUILDDIR="$1"

if [ ! -d "$BUILDDIR" ]; then
  cat - << EOF
The specified build directory does not exist. You must create it first.
EOF
  exit 1
fi

sudo vde_switch -daemon -s "$BUILDDIR/example.com"
sudo vde_switch -daemon -s "$BUILDDIR/example.org"

sudo tunctl -t tap27
sudo ip addr add 192.168.27.1/24 dev tap27
sudo ip link set tap27 up

sudo sed -i 's/^#net\.ipv4\.ip_forward.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sudo sysctl -p
# -o should use your publically facing interface
sudo iptables -t nat -A POSTROUTING -o br0 -j MASQUERADE
sudo iptables -I FORWARD -i tap27 -j ACCEPT
sudo iptables -I FORWARD -o tap27 -j ACCEPT

