#!/bin/bash

# $1 - container
# $2 - container veth interface
# $3 - VLAN
# $4 - ip/subnet declaration - e.g. 100.64.0.2/24

cdr2mask ()
{
   # Number of args to shift, 255..255, first non-255 byte, zeroes
   set -- $(( 5 - ($1 / 8) )) 255 255 255 255 $(( (255 << (8 - ($1 % 8))) & 255 )) 0 0 0
   [ $1 -gt 1 ] && shift $1 || shift
   echo ${1-0}.${2-0}.${3-0}.${4-0}
}

CONTAINER="$1"
INTERFACE="$2"
VLAN="$3"
IP=`echo "$4" | awk -F'/' '{print $1}'`
SUBNET_LEN=`echo "$4" | awk -F'/' '{print $2}'` 
SUBNET_MASK=`cdr2mask "$SUBNET_LEN"`

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
# Define the interface underneath /etc/network/interfaces.d
cat > /etc/network/interfaces.d/$INTERFACE.$VLAN << FOE
auto $INTERFACE.$VLAN
iface $INTERFACE.$VLAN inet static
    address $IP
    netmask $SUBNET_MASK
	vlan-raw-default $INTERFACE
FOE
ifup $INTERFACE.$VLAN
EOF
