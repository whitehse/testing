#!/bin/bash

# $1 - container
# $2 - container veth interface
# $3 - ip/subnet declaration - e.g. 100.64.0.2/24
# $4 - gateway

cdr2mask ()
{
   # Number of args to shift, 255..255, first non-255 byte, zeroes
   set -- $(( 5 - ($1 / 8) )) 255 255 255 255 $(( (255 << (8 - ($1 % 8))) & 255 )) 0 0 0
   [ $1 -gt 1 ] && shift $1 || shift
   echo ${1-0}.${2-0}.${3-0}.${4-0}
}

CONTAINER="$1"
INTERFACE="$2"
IP=`echo "$3" | awk -F'\/' '{print $1}'`
SUBNET_LEN=`echo "$3" | awk -F'\/' '{print $2}'` 
SUBNET_MASK=`cdr2mask "$SUBNET_LEN"`
GATEWAY="$4"

# Remove the default interface definition if it exists.

sed -i "/$INTERFACE/d" /etc/network/interfaces

# Define the interface underneath /etc/network/interfaces.d

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
ifdown $INTERFACE
ifconfig $INTERFACE 0
cat > /etc/network/interfaces.d/$INTERFACE << FOE
auto eth0
iface eth0 inet static
    address $IP
    netmask $SUBNET_MASK
    gateway $GATEWAY
FOE
ifup $INTERFACE
apt-get update
apt-get -y install bird bird-bgp iputils-ping gzip wget less vim
EOF

