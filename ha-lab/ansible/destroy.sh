#!/bin/bash

for foo in `cat hosts | grep '^[a-z]' | awk '{print $1}'`
do
  sudo lxc-stop -n $foo
  sudo lxc-destroy -n $foo
done

for foo in `grep link host_vars/* | awk '{print $3}' | sort | uniq`
do
  sudo ip link set $foo down
  sudo ip link delete $foo type bridge
done
