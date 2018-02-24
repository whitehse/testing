#!/bin/bash

ansible-playbook --connection=local -i hosts site-containers.yml

for foo in host_vars/*
do
  sudo lxc-stop -n `basename $foo`
  sudo lxc-start -n `basename $foo`
done

sudo lxc-attach -n isp -- ifconfig eth24 192.168.100.2/30
sudo lxc-attach -n isp -- route add default gw 192.168.100.1
sudo lxc-attach -n isp -- vtysh -c 'config t
router bgp
address-family ipv4 unicast
network 0.0.0.0/0'
sudo ifconfig bruplink_24 192.168.100.1/30
sudo route add -net 192.0.2.0/24 gw 192.168.100.2

for foo in host_vars/*
do
  sudo lxc-attach -n `basename $foo` -- bash -l -s << EOF
echo "nameserver 8.8.8.8" > /etc/resolv.conf
echo "root:passw0rd" | chpasswd
apt-get -y install openssh-server
sed -i "s/^#PermitRoot.*/PermitRootLogin yes/" /etc/ssh/sshd_config
service ssh reload
EOF
done

#ansible-playbook -i hosts -e "ansible_user=root ansible_ssh_pass=passw0rd ansible_sudo_pass:passw0rd" site.yml
export ANSIBLE_HOST_KEY_CHECKING=False
ansible-playbook -i hosts -e "ansible_user=root ansible_ssh_pass=passw0rd host_key_checking=False" site.yml

exit 0
