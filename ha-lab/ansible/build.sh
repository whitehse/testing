#!/bin/bash

#ansible-playbook --connection=local -i hosts clonable-containers.yml
ansible-playbook --connection=local -i hosts site-containers.yml

for foo in host_vars/*
do
  sudo lxc-stop -n `basename $foo`
  sudo lxc-start -n `basename $foo`
done
