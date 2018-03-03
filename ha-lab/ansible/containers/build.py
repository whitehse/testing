#!/usr/bin/python

import re
import sys
import os
from subprocess import call

filename = "../topology.dot"
file = open(filename, "r")

# Create network namespaces
for line in file:
    if re.search("eth[0-9]", line):
        line = line.replace('"','').replace(' ','').rstrip()
        src, dst = line.split("--")
        src_host, src_port = src.split(':')
        dst_host, dst_port = dst.split(':')

        if "host" in src_host:
            if call("sudo ip netns exec " + dst_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("sudo ip netns add " + dst_host, shell=True)
            call("sudo ip link add " + src_port + " type veth peer name " + dst_port + " netns " + dst_host, shell=True)
        else:
            if call("sudo ip netns exec " + src_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("sudo ip netns add " + src_host, shell=True)
            if call("sudo ip netns exec " + dst_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("sudo ip netns add " + dst_host, shell=True)
            call("sudo ip link add " + src_port + " netns " + src_host + " type veth peer name " + dst_port + " netns " + dst_host, shell=True)

file.close()
file = open(filename, "r")

allowed_devices = ["c 5:1" "c 1:3" "c 1:5" "c 1:8" "c 1:9"]

# Create cgroups
for line in file:
    if re.search("eth[0-9]", line):
        line = line.replace('"','').replace(' ','').rstrip()
        src, dst = line.split("--")
        src_host, src_port = src.split(':')
        dst_host, dst_port = dst.split(':')

        if "host" not in src_host:
            if not os.path.isdir("/sys/fs/cgroup/memory/" + src_host):
                call("sudo cgcreate -g cpu,memory,blkio,devices,freezer:/" + src_host, shell=True)
                call("sudo cgset -r memory.limit_in_bytes=500M " + src_host, shell=True)
                call("sudo cgset -r devices.deny=a " + src_host, shell=True)
                for device in allowed_devices:
                    call("sudo cgset -r devices.allow=\"$d rw\" " + src_host, shell=True)

        if not os.path.isdir("/sys/fs/cgroup/memory/" + dst_host):
            call("sudo cgcreate -g cpu,memory,blkio,devices,freezer:/" + dst_host, shell=True)
            call("sudo cgset -r memory.limit_in_bytes=500M " + dst_host, shell=True)
            call("sudo cgset -r devices.deny=a " + dst_host, shell=True)
            for device in allowed_devices:
                call("sudo cgset -r devices.allow=\"$d rw\" " + dst_host, shell=True)

#for foo in `sudo ip netns list`; do   sudo ip netns delete $foo; done
#HOSTS=`mktemp`
#cat "$TOPOLOGY" | sed 's/#.*//' | awk '{print $1}' | grep eth | awk -F':' '{print $1}' | sed 's/\"//g' | sort | uniq > $HOSTS
#cat "$TOPOLOGY" | sed 's/*.*//' | awk '{print $3}' | grep eth | awk -F':' '{print $1}' | sed 's/\"//g' | sort | uniq >> $HOSTS
#for HOST in `cat $HOSTS | sort | uniq`
#do
#  sudo cgdelete -g cpu,memory,blkio,devices,freezer:/${HOST}
#done

file.close()
file = open(filename, "r")

#sudo cgdelete -g cpu,memory,blkio,devices,freezer:/isp

#sudo cgexec -g cpu,memory,blkio,devices,freezer:/isp       \
#  prlimit --nofile=256 --nproc=512 --locks=32         \
#    ip netns exec isp                                 \
#      unshare -i -m -u -C -p -f --mount-proc=/proc    \
#        chroot /var/cache/lxc/debian-bgp-ecmp/rootfs-stable-amd64/ /bin/dash
