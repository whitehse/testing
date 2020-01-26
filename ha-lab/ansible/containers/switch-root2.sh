#!/bin/sh

mount --bind $1 /mnt
mount --make-private /mnt
#touch /mnt/mount
#mount proc /mnt/proc -t proc
#mount devpts /mnt/dev/pts -t devpts
unshare -i -u -p -m --propagation private --mount-proc=/proc -f switch_root /mnt /sbin/init 2
#exec switch_root /mnt /sbin/init 2

