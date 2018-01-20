#!/bin/bash

# $1 - Container
# $2 - Interfaces to listen on, in this form:
#
# install-common-config.sh router-a '"eth0", "eth1"'

CONTAINER="$1"
INTERFACES="$2"

ROUTER_ID=`ip address show dev lo:0 | grep 'inet ' | awk '{print $2}' | awk -F'/' '{print $1}'`

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
FOE
/etc/init.d/bird reload
EOF
