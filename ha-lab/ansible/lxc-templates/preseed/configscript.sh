#!/bin/bash

export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true
export LC_ALL=C LANGUAGE=C LANG=C
#/var/lib/dpkg/info/slapd.preinst install
if [ -d /tmp/preseeds/ ]; then
        for file in `ls -1 /tmp/preseeds/*`; do
        debconf-set-selections $file
        done
fi
mount proc -t proc /proc
mknod ./dev/random c 1 8
chmod 640 ./dev/random
chown 0:0 ./dev/random
mknod ./dev/urandom c 1 9
chmod 640 ./dev/urandom
chown 0:0 ./dev/urandom
/etc/init.d/nslcd stop
update-rc.d nslcd disable
dpkg --configure -a
echo "nameserver 8.8.8.8" > /etc/resolv.conf
echo "root:passw0rd" | chpasswd
sed -i "s/^#PermitRoot.*/PermitRootLogin yes/" /etc/ssh/sshd_config
service ssh reload
umount proc
