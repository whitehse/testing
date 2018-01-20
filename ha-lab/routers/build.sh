#!/bin/bash

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n router-a
sudo cp router-a.config /var/lib/lxc/router-a/config
sudo lxc-start -n router-a
sudo ifconfig router-a-0 100.64.0.1/30

sleep 1
../common/utils/set-primary-v4-address.sh router-a eth0 100.64.0.2/30 100.64.0.1
../common/utils/install-packages.sh router-a bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan
../common/bird/install-common-config.sh router-a 100.64.0.2

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n router-b
sudo cp router-b.config /var/lib/lxc/router-b/config
sudo lxc-start -n router-b
sudo ifconfig router-b-0 100.64.0.5/30

sleep 1
../common/utils/set-primary-v4-address.sh router-b eth0 100.64.0.6/24 100.64.0.5
../common/utils/install-packages.sh router-b bird,bird-bgp,iputils-ping,gzip,wget,less,vim,vlan
../common/bird/install-common-config.sh router-b 100.64.0.6

#VLANs:
#100 - slapd-1       - 100.64.0.8/30
#101 - postgres-1    - 100.64.0.12/30
#102 - mit-auth-1    - 100.64.0.16/30
#103 - postfix-1     - 100.64.0.20/30
#104 - cyrus-fe-1    - 100.64.0.24/30
#105 - cyrus-be-1-1  - 100.64.0.28/30
#106 - cyrus-be-2-1  - 100.64.0.32/30
#107 - nfs-1         - 100.64.0.36/30
#108 - nfs-backup-1  - 100.64.0.40/30
#109 - cyrus-mup-1   - 100.64.0.44/30
#110 - nginx-1       - 100.64.0.48/30
#111 - freeswitch-1  - 100.64.0.52/30
#112 - bind-1        - 100.64.0.56/30
#  2 - router-b      - 100.64.0.112/30
#
#200 - slapd-2       - 100.64.0.60/30
#201 - postgres-2    - 100.64.0.64/30
#202 - mit-auth-2    - 100.64.0.68/30
#203 - postfix-2     - 100.64.0.72/30
#204 - cyrus-fe-2    - 100.64.0.76/30
#205 - cyrus-be-1-2  - 100.64.0.80/30
#206 - cyrus-be-2-2  - 100.64.0.84/30
#207 - nfs-2         - 100.64.0.88/30
#208 - nfs-backup-2  - 100.64.0.92/30
#209 - cyrus-mup-2   - 100.64.0.96/30
#210 - nginx-2       - 100.64.0.100/30
#211 - freeswitch-2  - 100.64.0.104/30
#212 - bind-2        - 100.64.0.108/30
#  2 - router-a      - 100.64.0.112/30

# Host Loopbacks
# Host router  - 100.64.1.0/32
# router-a     - 100.64.1.1/32
# router-b     - 100.64.1.2/32
# slapd-1      - 100.64.1.3/32
# postgres-1   - 100.64.1.4/32
# mit-auth-1   - 100.64.1.5/32
# postfix-1    - 100.64.1.6/32
# cyrus-fe-1   - 100.64.1.7/32
# cyrus-be-1-1 - 100.64.1.8/32
# cyrus-be-2-1 - 100.64.1.9/32
# nfs-1        - 100.64.1.10/32
# nfs-backup-1 - 100.64.1.11/32
# cyrus-mup-1  - 100.64.1.12/32
# nginx-1      - 100.64.1.13/32
# freeswitch-1 - 100.64.1.14/32
# bind-1       - 100.64.1.15/32
# slapd-2      - 100.64.1.16/32
# postgres-2   - 100.64.1.17/32
# mit-auth-2   - 100.64.1.18/32
# postfix-2    - 100.64.1.19/32
# cyrus-fe-2   - 100.64.1.20/32
# cyrus-be-1-2 - 100.64.1.21/32
# cyrus-be-2-2 - 100.64.1.22/32
# nfs-2        - 100.64.1.23/32
# nfs-backup-2 - 100.64.1.24/32
# cyrus-mup-2  - 100.64.1.25/32
# nginx-2      - 100.64.1.26/32
# freeswitch-2 - 100.64.1.27/32
# bind-2       - 100.64.1.28/32
#
# Service Loopbacks
# ldap.example.com       - 100.64.1.100/32
# kdc.example.com        - 100.64.1.101/32
# postgres.example.com   - 100.64.1.102/32
# auth.example.com       - 100.64.1.103/32
# smtp.example.com       - 100.64.1.104/32
# imap.example.com       - 100.64.1.105/32
# imap-be-1.example.com  - 100.64.1.106/32
# imap-be-2.example.com  - 100.64.1.107/32
# nfs.example.com        - 100.64.1.108/32
# nfs-backup.example.com - 100.64.1.109/32
# mupdate.example.com    - 100.64.1.110/32
# www.example.com        - 100.64.1.111/32
# voice.example.com      - 100.64.1.112/32
# ns1.example.com        - 100.64.1.113/32
# ns2.example.com        - 100.64.1.114/32

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

