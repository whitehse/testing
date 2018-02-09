#!/bin/sh

set -e

echo '!!!Beging script!!!'
export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true
export LC_ALL=C LANGUAGE=C LANG=C
#/var/lib/dpkg/info/slapd.preinst install
echo '!!!Starting seed loop!!!'
if [ -d /tmp/preseeds/ ]; then
        for file in `ls -1 /tmp/preseeds/*`; do
        debconf-set-selections $file
        done
#echo -n "adminpw = "
#db_get slapd/internal/adminpw
#echo "$RET"
fi
echo '!!!Finished seed loop!!!'
dpkg --configure -a
mount proc -t proc /proc
dpkg --configure -a
umount proc
