#!/bin/bash

# $1 - container
# $2 - container lo interface - e.g. lo:0
# $3 - ip/subnet declaration - e.g. 100.64.1.0/32

cdr2mask ()
{
   # Number of args to shift, 255..255, first non-255 byte, zeroes
   set -- $(( 5 - ($1 / 8) )) 255 255 255 255 $(( (255 << (8 - ($1 % 8))) & 255 )) 0 0 0
   [ $1 -gt 1 ] && shift $1 || shift
   echo ${1-0}.${2-0}.${3-0}.${4-0}
}

CONTAINER="$1"
INTERFACE="$2"
IP=`echo "$3" | awk -F'/' '{print $1}'`
SUBNET_LEN=`echo "$3" | awk -F'/' '{print $2}'` 
SUBNET_MASK=`cdr2mask "$SUBNET_LEN"`

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
# Add source for directory inclusion
grep -q lo /etc/network/interfaces
if [ $? -eq 0 ]; then
  echo "source /etc/network/interfaces.d/*" > /etc/network/interfaces
  cat > /etc/network/interfaces.d/lo << FOE
auto lo
iface lo inet loopback

FOE
fi
cat > /etc/network/interfaces.d/$INTERFACE << FOE
auto $INTERFACE
iface $INTERFACE inet static
    address $IP
    netmask $SUBNET_MASK
FOE
ifup $INTERFACE
EOF
