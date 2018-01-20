#!/bin/bash

# $1 - Container
# $2 - Interfaces to listen on, in this form:
# $3 - Loopback/Router ID
#
# install-common-config.sh router-a '"eth0", "eth1"'

CONTAINER="$1"
INTERFACES="$2"
ROUTER_ID="$3"

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
if [ ! -d /etc/bird/bird.d ]; then
	mkdir /etc/bird/bird.d
fi

cat > /etc/bird/bird.conf << FOE
include "/etc/bird/bird.d/*";

router id $ROUTER_ID;

log syslog { debug, trace, info, remote, warning, error, auth, fatal, bug };
#log stderr all;
#log "/tmp/bird.log" all;

protocol device {
}

protocol static {
}

protocol direct {
        interface $INTERFACES;
}

protocol kernel {
    metric 64;
    export all;
}

protocol bfd BFD1 {
    debug { states, routes, filters, interfaces, events };
    interface $INTERFACES {
        min rx interval 50 ms;
        min tx interval 50 ms;
        #idle tx interval 1000 ms;
        multiplier 3;
    };
}

template bgp ibgp {
       local as 65000;
       source address $ROUTER_ID;
       bfd on;
       next hop self;
	   import all;
	   export all;
}

FOE
/etc/init.d/bird reload
EOF
