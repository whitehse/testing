#!/usr/bin/python

import re
import sys
import os
from subprocess import *
from jinja2 import Environment, FileSystemLoader

env = Environment(loader=FileSystemLoader("templates"))

filename = "../topology.dot"
file = open(filename, "r")

hosts = {}

# Create network namespaces
for line in file:
    if re.search("eth[0-9]", line):
        line = line.replace('"','').replace(' ','').rstrip()
        src, dst = line.split("--")
        src_host, src_port = src.split(':')
        dst_host, dst_port = dst.split(':')

        if "host" in src_host:
            if dst_host not in hosts:
                hosts[dst_host] = []
            if dst_port not in hosts[dst_host]:
                hosts[dst_host].append(dst_port)
            if call("ip netns exec " + dst_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("ip netns add " + dst_host, shell=True)
            call("ip link add " + src_port + " type veth peer name " + dst_port + " netns " + dst_host, shell=True)
            call("ip link set dev " + src_port + " up", shell=True)
            call("ip netns exec " + dst_host + " ip link set dev " + dst_port + " up", shell=True)
        else:
            if src_host not in hosts:
                hosts[src_host] = []
            if src_port not in hosts[src_host]:
                hosts[src_host].append(src_port)
            
            if dst_host not in hosts:
                hosts[dst_host] = []
            if dst_port not in hosts[dst_host]:
                hosts[dst_host].append(dst_port)

            if call("ip netns exec " + src_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("ip netns add " + src_host, shell=True)
            if call("ip netns exec " + dst_host + " ip link list > /dev/null 2> /dev/null", shell=True):
                call("ip netns add " + dst_host, shell=True)
            call("ip link add " + src_port + " netns " + src_host + " type veth peer name " + dst_port + " netns " + dst_host, shell=True)
            call("ip netns exec " + src_host + " ip link set dev " + src_port + " up", shell=True)
            call("ip netns exec " + dst_host + " ip link set dev " + dst_port + " up", shell=True)

file.close()

allowed_devices = ["c 5:1", "c 1:3", "c 1:5", "c 1:8", "c 1:9"]

asn_counter=4200000000
loopback_counter=0

for host in hosts:
    call("cgcreate -g cpu,memory,blkio,devices,freezer:/" + host, shell=True)
    call("cgset -r memory.limit_in_bytes=500M " + host, shell=True)
    call("cgset -r devices.deny=a " + host, shell=True)
    for device in allowed_devices:
        call("cgset -r devices.allow=\"" + device + " rw\" " + host, shell=True)

    call("ip netns exec " + host + " ip link set dev lo up", shell=True)
    call("ip netns exec " + host + " ip addr add 192.0.2." + str(loopback_counter) + "/32 dev lo", shell=True)

    call("btrfs subvolume snapshot build build." + host, shell=True)

    bgpconfig = "build." + host + "/etc/frr/bgpd.conf"
    template = env.get_template('bgpd.j2')
    f = open(bgpconfig, 'w' )
    #for interface in hosts[host]:
    #    print "host=" + host + ". interface = " + interface
    f.write(template.render(asn=asn_counter, router_id="192.168.0." + str(loopback_counter), interfaces=hosts[host]))
    f.close()

    Popen("cgexec -g cpu,memory,blkio,devices,freezer:/" + host + " prlimit --nofile=256 --nproc=512 --locks=32 ip netns exec " + host + " unshare -i -u -C -p -f --mount-proc=/proc --fork ./switch-root.sh build." + host, shell=True)

    asn_counter+=1
    loopback_counter+=1
