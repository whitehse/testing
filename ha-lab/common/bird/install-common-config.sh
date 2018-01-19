#!/bin/bash

# $1 - Container
# $2 - router-id

CONTAINER="$1"
ROUTER_ID="$2"

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
if [ ! -d /etc/bird/bird.d ]; then
	mkdir /etc/bird/bird.d
fi

cat > /etc/bird/bird.conf << FOE
include "/etc/bird/bird.d/*";

router id $ROUTER_ID;

protocol device {
}

# The Kernel protocol is not a real routing protocol. Instead of communicating
# with other routers in the network, it performs synchronization of BIRD's
# routing tables with the OS kernel.
protocol kernel {
        metric 64;      # Use explicit kernel route metric to avoid collisions
                        # with non-BIRD routes in the kernel routing table
        import all;
       export all;     # Actually insert routes into the kernel routing table
}

FOE
/etc/init.d/bird reload
EOF
