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
The specified build directory does not exist. You must create it first.
EOF
  exit 1
fi

for foo in user-mode-linux uml-utilities vde2 debootstrap
do
  verify-package-installation "$foo"
done

if [ -d "$BUILDDIR/firewall" ]; then
  cat - << EOF
The Firewall image already exists. Nothing to create.
EOF
  exit 1
fi

if [ -f "$BUILDDIR/firewall.img" ]; then
  cat - << EOF
The Firewall image already exists. Nothing to create.
EOF
  exit 1
fi

echo "Creating firewall image"

mkdir "$BUILDDIR/firewall"

truncate -s 10G "$BUILDDIR/firewall.img"
/sbin/mke2fs -t ext3 "$BUILDDIR/firewall.img"

sudo mount "$BUILDDIR/firewall.img" "$BUILDDIR/firewall"
sudo debootstrap --arch amd64 --variant minbase --include sysvinit-core,kmod,mingetty,less,vim,sudo,git,wget,openssh-server,ifupdown,iproute2,iputils-ping,iptables,iptables-persistent,bind9,dnsutils,openssl,nc stable "$BUILDDIR/firewall" http://ftp.us.debian.org/debian/
sudo rsync -r \
  --chown=root:root \
  firewall/* "$BUILDDIR/firewall/"
sudo mkdir -p "$BUILDDIR/firewall/lib/modules"
sudo cp -a /usr/lib/uml/modules/* "$BUILDDIR/firewall/lib/modules/"
install-public-ssh-key "$BUILDDIR/firewall/"
sudo umount "$BUILDDIR/firewall"
