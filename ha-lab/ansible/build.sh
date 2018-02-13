#!/bin/bash

#ansible-playbook --connection=local -i hosts clonable-containers.yml
ansible-playbook --connection=local -i hosts site-containers.yml

for foo in host_vars/*
do
  sudo lxc-stop -n `basename $foo`
  sudo lxc-start -n `basename $foo`
done

sudo lxc-attach -n spine-a.z -- ifconfig eth24 192.168.100.2
sudo lxc-attach -n spine-a.z -- route add default gw 192.168.100.1
sudo ifconfig bruplink_42 192.168.100.1/24
sudo route add -net 192.0.2.0/24 gw 192.168.100.2
