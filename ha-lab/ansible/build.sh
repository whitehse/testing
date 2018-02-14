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
sudo lxc-attach -n spine-a.z -- vtysh -c 'config t
router bgp
address-family ipv4 unicast
network 0.0.0.0/0'
sudo ifconfig bruplink_42 192.168.100.1/24
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

exit 0

cat > /tmp/slapd.preseed << FOE
#_preseed_V1
slapd   slapd/password1 password        passw0rd
slapd   slapd/internal/generated_adminpw        password
slapd   slapd/internal/adminpw  password
slapd   slapd/password2 password        passw0rd
slapd   slapd/domain    string  example.org
slapd   slapd/dump_database     select  when needed
slapd   shared/organization     string  example.org
slapd   slapd/unsafe_selfwrite_acl      note
slapd   slapd/no_configuration  boolean false
slapd   slapd/backend   select  MDB
slapd   slapd/move_old_database boolean true
slapd   slapd/invalid_config    boolean true
slapd   slapd/ppolicy_schema_needs_update       select  abort installation
slapd   slapd/dump_database_destdir     string  /var/backups/slapd-VERSION
slapd   slapd/password_mismatch note
slapd   slapd/purge_database    boolean false
FOE
debconf-set-selections /tmp/slapd.preseed

