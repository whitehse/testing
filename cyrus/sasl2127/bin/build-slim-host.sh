#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A build directory must be specified.
EOF
  exit 1
fi

if [ -z "$2" ]; then
  cat - << EOF
A hostname must be provided.
EOF
  exit 1
fi

export BUILDDIR="$1"
export HOSTNAME="$2"

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

if [ -d "$BUILDDIR/${HOSTNAME}" ]; then
  cat - << EOF
The Firewall image already exists. Nothing to create.
EOF
  exit 1
fi

if [ -f "$BUILDDIR/${HOSTNAME}.img" ]; then
  cat - << EOF
The Firewall image already exists. Nothing to create.
EOF
  exit 1
fi

echo "Creating ${HOSTNAME} image"

mkdir "$BUILDDIR/${HOSTNAME}"

truncate -s 10G "$BUILDDIR/${HOSTNAME}.img"
/sbin/mke2fs -t ext3 "$BUILDDIR/${HOSTNAME}.img"

sudo mount "$BUILDDIR/${HOSTNAME}.img" "$BUILDDIR/${HOSTNAME}"
sudo debootstrap --arch amd64 --variant minbase --include sysvinit-core,kmod,mingetty,less,vim,wget,dropbear-run,ifupdown,iputils-ping stable "$BUILDDIR/${HOSTNAME}" http://ftp.us.debian.org/debian/
sudo rsync -r \
  --chown=root:root \
  ${HOSTNAME}/* "$BUILDDIR/${HOSTNAME}/"
sudo mkdir -p "$BUILDDIR/${HOSTNAME}/lib/modules"
sudo cp -a /usr/lib/uml/modules/* "$BUILDDIR/${HOSTNAME}/lib/modules/"
install-public-ssh-key "$BUILDDIR/${HOSTNAME}/"
sudo cp /usr/bin/scp "$BUILDDIR/${HOSTNAME}/usr/bin/"
sudo chroot "$BUILDDIR/${HOSTNAME}/" /bin/bash -c "echo root:passw0rd | chpasswd"
sudo umount "$BUILDDIR/${HOSTNAME}"
