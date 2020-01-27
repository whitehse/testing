#!/usr/bin/python

import re
import sys
import os
from subprocess import *

filename = "../topology-test.dot"
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

allowed_devices = ["c 5:1", "c 1:3", "c 1:5", "c 1:8", "c 1:9"]

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
                    call("sudo cgset -r devices.allow=\"" + device + " rw\" " + src_host, shell=True)

        if not os.path.isdir("/sys/fs/cgroup/memory/" + dst_host):
            call("sudo cgcreate -g cpu,memory,blkio,devices,freezer:/" + dst_host, shell=True)
            call("sudo cgset -r memory.limit_in_bytes=500M " + dst_host, shell=True)
            call("sudo cgset -r devices.deny=a " + dst_host, shell=True)
            for device in allowed_devices:
                call("sudo cgset -r devices.allow=\"" + device + " rw\" " + dst_host, shell=True)

file.close()

filename = "hosts-test"
file = open(filename, "r")

for line in file:
    host = line.rstrip()
    call("sudo btrfs subvolume snapshot build build." + host, shell=True)
    #call("sudo cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -m -u -C -p -f --mount-proc=/proc --fork chroot build." + host + " /usr/local/bin/init&", shell=True)
    #call("sudo ip netns exec " + host + " unshare -i -m -u -C -p -f --mount-proc=/proc --fork chroot build." + host + " bash", shell=True)
    Popen("sudo cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -u -C -p -f --mount-proc=/proc --fork ./switch-root.sh build." + host, shell=True)

#        mount -t tmpfs none /home
#        mount -t tmpfs none /tmp
#        mount -t tmpfs none /sys
#        mount -t tmpfs none /var/log
#        exec su -l ${SUDO_USER}
#      "
