#!/bin/bash

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n slapd-1
sudo cp slapd-1.config /var/lib/lxc/slapd-1/config
sudo lxc-start -n slapd-1

sudo ovs-vsctl add-port switch-a slapd-1-0 tag=100

sleep 1

../common/utils/set-preseed.sh slapd-1 preseed

#100 - slapd-1       - 100.64.0.8/30
# slapd-1      - 100.64.1.3/32
../common/utils/set-loopback-address.sh slapd-1 lo:0 100.64.1.3/32
# Temporarily configure gateway to configure the system
../common/utils/set-primary-v4-address.sh slapd-1 eth0 100.64.0.10/30 100.64.0.9
../common/utils/install-packages.sh slapd-1 bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan,slapd,ldap-utils
../common/bird/install-common-config.sh slapd-1 '"eth0"' 100.64.1.3


sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n slapd-2
sudo cp slapd-2.config /var/lib/lxc/slapd-2/config
sudo lxc-start -n slapd-2

sudo ovs-vsctl add-port switch-b slapd-2-0 tag=200

sleep 1

../common/utils/set-preseed.sh slapd-2 preseed

#200 - slapd-2       - 100.64.0.60/30
# slapd-2      - 100.64.1.16/32
../common/utils/set-loopback-address.sh slapd-2 lo:0 100.64.1.16/32
# Temporarily configure gateway to configure the system
../common/utils/set-primary-v4-address.sh slapd-2 eth0 100.64.0.62/30 100.64.0.61
../common/utils/install-packages.sh slapd-2 bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan,slapd,ldap-utils
../common/bird/install-common-config.sh slapd-2 '"eth0"' 100.64.1.16

../common/bird/install-config.sh slapd-1 ospf-slapd-1.conf
../common/bird/install-config.sh slapd-2 ospf-slapd-2.conf

../common/bird/install-config.sh slapd-1 bgp-slapd-1.conf
../common/bird/install-config.sh slapd-2 bgp-slapd-2.conf

../common/utils/set-primary-v4-address.sh slapd-1 eth0 100.64.0.10/30
../common/utils/set-primary-v4-address.sh slapd-2 eth0 100.64.0.62/24

../common/bird/start-bird.sh slapd-1
../common/bird/start-bird.sh slapd-1

exit 0




sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n router-a
sudo cp router-a.config /var/lib/lxc/router-a/config
sudo lxc-start -n router-a
sudo ifconfig router-a-0 100.64.0.1/30

sleep 1
# router-a     - 100.64.1.1/32
../common/utils/set-loopback-address.sh router-a lo:0 100.64.1.1/32
# Temporarily add a default route
../common/utils/set-primary-v4-address.sh router-a eth0 100.64.0.2/30 100.64.0.1
../common/utils/install-packages.sh router-a bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan
../common/bird/install-common-config.sh router-a '"eth0", "lo:0", "eth1.100", "eth1.101", "eth1.102", "eth1.103", "eth1.104", "eth1.105", "eth1.106", "eth1.107", "eth1.108", "eth1.109", "eth1.110", "eth1.111", "eth1.112"' 100.64.1.1

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n router-b
sudo cp router-b.config /var/lib/lxc/router-b/config
sudo lxc-start -n router-b
sudo ifconfig router-b-0 100.64.0.5/30

sleep 1
# router-b     - 100.64.1.2/32
../common/utils/set-loopback-address.sh router-b lo:0 100.64.1.2/32
# Temporarily add a default route
../common/utils/set-primary-v4-address.sh router-b eth0 100.64.0.6/24 100.64.0.5
../common/utils/install-packages.sh router-b bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan
../common/bird/install-common-config.sh router-b '"eth0", "lo:0", "eth1.200", "eth1.201", "eth1.202", "eth1.203", "eth1.204", "eth1.205", "eth1.206", "eth1.207", "eth1.208", "eth1.209", "eth1.210", "eth1.211", "eth1.212"' 100.64.1.2

sudo ovs-vsctl add-port switch-a router-a-1
sudo ovs-vsctl add-port switch-b router-b-1

sudo ovs-vsctl set port router-a-1 trunks=100,101,102,103,104,105,106,107,108,109,110,111,112
sudo ovs-vsctl set port router-b-1 trunks=200,201,202,203,204,205,206,207,208,209,210,211,212

../common/utils/enable-forwarding.sh router-a
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 100 100.64.0.9/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 101 100.64.0.13/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 102 100.64.0.17/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 103 100.64.0.21/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 104 100.64.0.25/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 105 100.64.0.29/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 106 100.64.0.33/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 107 100.64.0.37/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 108 100.64.0.41/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 109 100.64.0.45/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 110 100.64.0.49/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 111 100.64.0.53/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth1 112 100.64.0.57/30
../common/utils/set-primary-vlan-v4-address.sh router-a eth2 2   100.64.0.113/30

../common/utils/enable-forwarding.sh router-b
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 200 100.64.0.61/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 201 100.64.0.65/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 202 100.64.0.69/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 203 100.64.0.73/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 204 100.64.0.77/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 205 100.64.0.81/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 206 100.64.0.85/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 207 100.64.0.89/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 208 100.64.0.93/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 209 100.64.0.97/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 210 100.64.0.101/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 211 100.64.0.105/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth1 212 100.64.0.109/30
../common/utils/set-primary-vlan-v4-address.sh router-b eth2 2   100.64.0.114/30

../common/bird/install-config.sh router-a ospf-router-a.conf
../common/bird/install-config.sh router-b ospf-router-b.conf

../common/bird/install-config.sh router-a bgp-router-a.conf
../common/bird/install-config.sh router-b bgp-router-b.conf

../common/utils/set-primary-v4-address.sh router-a eth0 100.64.0.2/30
../common/utils/set-primary-v4-address.sh router-b eth0 100.64.0.6/24

../common/bird/start-bird.sh router-a
../common/bird/start-bird.sh router-b

