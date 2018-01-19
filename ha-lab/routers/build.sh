#!/bin/bash

sudo lxc-create -t /usr/share/lxc/templates/lxc-debian -n router-a
sudo cp router-a.config /var/lib/lxc/router-a/config
sudo lxc-start -n router-a
sudo ifconfig router-a-uplink 100.64.0.1/24

../common/utils/set-primary-v4-address.sh router-a eth0 100.64.0.2/24 100.64.0.1
../common/utils/install-packages.sh router-a bird,bird-bgp,iputils-ping,gzip,wget,less,vim
../common/bird/install-common-config.sh router-a 100.64.0.2
