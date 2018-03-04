#!/bin/sh

mount --bind $1 /mnt
mount proc /mnt/proc -t proc
mount devpts /mnt/dev/pts -t devpts
exec switch_root /mnt /sbin/init 2
#cd /mnt
#exec pivot_root . mnt
#chroot . init 2
