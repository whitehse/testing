#!/usr/bin/python

import re
import sys
import os
from subprocess import *

filename = "../topology.dot"
file = open(filename, "r")

# Create network namespaces
for line in file:
    if re.search("eth[0-9]", line):
        line = line.replace('"','').replace(' ','').rstrip()
        src, dst = line.split("--")
        src_host, src_port = src.split(':')
        dst_host, dst_port = dst.split(':')

        #ip link set dev <interface> up

        if "host" in src_host:
            if call("sudo ip netns exec " + dst_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("sudo ip netns add " + dst_host, shell=True)
            call("sudo ip link add " + src_port + " type veth peer name " + dst_port + " netns " + dst_host, shell=True)
            call("sudo ip link set dev " + src_port + " up", shell=True)
            call("sudo ip netns exec " + dst_host + " ip link set dev " + dst_port + " up", shell=True)
        else:
            if call("sudo ip netns exec " + src_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("sudo ip netns add " + src_host, shell=True)
            if call("sudo ip netns exec " + dst_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("sudo ip netns add " + dst_host, shell=True)
            call("sudo ip link add " + src_port + " netns " + src_host + " type veth peer name " + dst_port + " netns " + dst_host, shell=True)
            call("sudo ip netns exec " + src_host + " ip link set dev " + src_port + " up", shell=True)
            call("sudo ip netns exec " + dst_host + " ip link set dev " + dst_port + " up", shell=True)

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

filename = "hosts"
file = open(filename, "r")

asn_counter=4200000000
loopback_counter=0

for line in file:
    host = line.rstrip()
    call("sudo ip netns exec " + host + " ip link set dev lo up", shell=True)
    call("sudo ip netns exec " + host + " ip addr add 192.0.2." + str(loopback_counter) + "/32 dev lo", shell=True)
    call("sudo btrfs subvolume snapshot build build." + host, shell=True)
    #Popen("sudo cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -m -u -C -p -f --mount-proc=/proc --fork chroot build." + host + " /sbin/init 2", shell=True)
    #Popen("sudo cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -u -C -p -f --mount-proc=/proc --fork chroot build." + host + " /sbin/init 2", shell=True)
    #Popen("sudo cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -u -C -p -f --mount-proc=/proc --fork mount --bind build." + host + " / ; switch_root /", shell=True)
    Popen("sudo cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -u -C -p -f --mount-proc=/proc --fork ./switch-root.sh build." + host, shell=True)
    loopback_counter+=1
