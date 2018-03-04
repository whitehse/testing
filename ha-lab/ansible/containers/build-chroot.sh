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

btrfs subvolume create "$BUILD_DIR"

multistrap -d "$BUILD_DIR" -f "$MULTISTRAP_CONF"
chroot "$BUILD_DIR" /configscript.sh
cp files/init.d-frr "$BUILD_DIR/etc/init.d/frr"
chmod 755 "$BUILD_DIR/etc/init.d/frr"
cp files/daemons* "$BUILD_DIR/etc/frr/"
cp files/interfaces "$BUILD_DIR/etc/network/"
cp files/zebra.conf "$BUILD_DIR/etc/frr/zebra.conf"
chroot build adduser frr frrvty
cat > "$BUILD_DIR/usr/local/bin/init" << FOE
#!/bin/sh

ASN=$RANDOM
cat > /etc/frr/bgpd.conf << EOF
log syslog
!
router bgp $ASN
 bgp router-id 192.0.2.234
 bgp bestpath as-path multipath-relax
 bgp bestpath compare-routerid
 neighbor fabric peer-group
 neighbor fabric remote-as external
 neighbor fabric description Internal Fabric Network
 neighbor fabric capability extended-nexthop

FOE


mount proc /proc -t proc
#mount --make-rprivate /
#service networking start
ifconfig lo up
for foo in `ifconfig -a | grep '^e' | awk '{print $1}' | sed 's/://g'`
do
  ifconfig $foo up
  echo " neighbor $foo interface peer-group fabric" >> /etc/frr/bgpd.conf
done
service ssh start
service slapd start
service bind9 start
service lldpd start
cat >> /etc/frr/bgpd.conf << FOE
!
 address-family ipv4 unicast
  redistribute connected
  maximum-paths 64
 exit-address-family
!
 address-family ipv6 unicast
  redistribute connected
  maximum-paths 64
 exit-address-family
!
line vty
FOE
service frr start
sleep 1000
EOF
chmod 755 "$BUILD_DIR/usr/local/bin/init"
#sudo cp -a build build.orig

# Left to do. Create /etc/frr/bgpd.conf, and the files
# underneath /etc/network/interfaces.d/
