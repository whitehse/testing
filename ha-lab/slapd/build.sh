#!/bin/bash

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n slapd-1
sudo cp slapd-1.config /var/lib/lxc/slapd-1/config
sudo lxc-start -n slapd-1

sudo ovs-vsctl add-port switch-a slapd-1-0 tag=100

sleep 1
#100 - slapd-1       - 100.64.0.8/30
# Temporarily configure gateway to configure the system
../common/utils/set-primary-v4-address.sh slapd-1 eth0 100.64.0.10/30 100.64.0.9
../common/utils/install-packages.sh slapd-1 bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan
../common/bird/install-common-config.sh slapd-1 100.64.0.10

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n slapd-2
sudo cp slapd-2.config /var/lib/lxc/slapd-2/config
sudo lxc-start -n slapd-2

sudo ovs-vsctl add-port switch-b slapd-2-0 tag=200

sleep 1
#200 - slapd-2       - 100.64.0.60/30
# Temporarily configure gateway to configure the system
../common/utils/set-primary-v4-address.sh slapd-2 eth0 100.64.0.62/30 100.64.0.61
../common/utils/install-packages.sh slapd-2 bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan
../common/bird/install-common-config.sh slapd-2 100.64.0.62

#../common/bird/install-config.sh slapd-1 ospf-slapd-1.conf
#../common/bird/install-config.sh slapd-2 ospf-slapd-2.conf

#../common/bird/install-config.sh slapd-1 bgp-slapd-1.conf
#../common/bird/install-config.sh slapd-2 bgp-slapd-2.conf

