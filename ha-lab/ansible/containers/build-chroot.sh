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

cat > "$BUILD_DIR/usr/local/bin/init" << EOF
#!/bin/sh

mount proc /proc -t proc
mount -t devpts -o gid=4,mode=620 none /dev/pts
#mount --make-rprivate /
#service networking start
#service ssh start
#service slapd start
#service bind9 start
#service lldpd start
#service frr start
#sleep 1000
/sbin/init 2
EOF
chmod 755 "$BUILD_DIR/usr/local/bin/init"
#sudo cp -a build build.orig

# Left to do. Create /etc/frr/bgpd.conf, and the files
# underneath /etc/network/interfaces.d/
