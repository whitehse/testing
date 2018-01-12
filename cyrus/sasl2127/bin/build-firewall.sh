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
sudo debootstrap --arch amd64 --variant minbase --include sysvinit-core,kmod,less,vim,sudo,git,wget,openssh-server,ifupdown,iproute2,iputils-ping,iptables,iptables-persistent,bind9,dnsutils,openssl stable "$BUILDDIR/firewall" http://ftp.us.debian.org/debian/
# Copy git config files over
#cp -r firewall/* "$BUILDDIR/firewall/"
sudo rsync -r \
  --chown=root:root \
  firewall/* "$BUILDDIR/firewall/"
sudo mkdir -p "$BUILDDIR/firewall/lib/modules"
sudo cp -a /usr/lib/uml/modules/* "$BUILDDIR/firewall/lib/modules/"
install-public-ssh-key "$BUILDDIR/firewall/"
sudo umount "$BUILDDIR/firewall"

#sudo linux ubd0="$BUILDDIR/firewall.img" mem=128M con=pty con0=fd:0,fd:1 eth0=tuntap,tap27,, eth1=vde,"$BUILDDIR/example.com" eth2=vde,"$BUILDDIR/example.org" umid=firewall

exit 0

sed -i 's/^#net\.ipv4\.ip_forward.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sysctl -p
#iptables -P OUTPUT ACCEPT
# -o should use your publically facing interface
iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
iptables -I FORWARD -i tap27 -j ACCEPT
iptables -I FORWARD -o tap27 -j ACCEPT

linux ubd0=firewall.img mem=128M con=pty con0=fd:0,fd:1 eth0=tuntap,tap27,f0:00:00:00:01,192.168.27.1 eth1=vde,./example.com eth2=vde,./example.org umid=firewall
ip addr add 192.168.27.2/24 dev eth0
ip link set eth0 up
ip route add default via 192.168.27.1 dev eth0
echo "root:passw0rd"|chpasswd
