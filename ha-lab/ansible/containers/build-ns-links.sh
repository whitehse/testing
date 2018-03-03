#!/bin/bash

#for foo in `sudo ip netns list`; do   sudo ip netns delete $foo; done

# "leaf-b.x":"eth3" -- "anteros.x":"eth1"
IFS='
'
for foo in `grep eth topology.dot`
do
    SRC_HOST=`echo "$foo" | awk '{print $1}' | awk -F':' '{print $1}' | sed 's/"//g'`
    SRC_INT=`echo "$foo" | awk '{print $1}' | awk -F':' '{print $2}' | sed 's/"//g'`
    DST_HOST=`echo "$foo" | awk '{print $3}' | awk -F':' '{print $1}' | sed 's/"//g'`
    DST_INT=`echo "$foo" | awk '{print $3}' | awk -F':' '{print $2}' | sed 's/"//g'`
    if [ $SRC_HOST == "host" ]; then
        sudo ip netns exec $DST_HOST ip link list > /dev/null 2> /dev/null  
        if [ $? -ne 0 ]; then
            sudo ip netns add "$DST_HOST"
        fi
        sudo ip link add $SRC_INT type veth peer name $DST_INT netns $DST_HOST
    else
        sudo ip netns exec $SRC_HOST ip link list > /dev/null 2> /dev/null  
        if [ $? -ne 0 ]; then
            sudo ip netns add "$SRC_HOST"
        fi
        sudo ip netns exec $DST_HOST ip link list > /dev/null 2> /dev/null  
        if [ $? -ne 0 ]; then
            sudo ip netns add "$DST_HOST"
        fi
        sudo ip link add $SRC_INT netns $SRC_HOST type veth peer name $DST_INT netns $DST_HOST
    fi
done

# From https://blogs.rdoproject.org/2015/08/hands-on-linux-sandbox-with-namespaces-and-cgroups/
sudo cgcreate -g cpu,memory,blkio,devices,freezer:/isp
# Set a limit of 2Gb
sudo cgset -r memory.limit_in_bytes=2G isp

# Get memory stats used by the cgroup
sudo cgget -r memory.stat isp

Limit to 1MB/s for read and write on common hard drive device
for dev in 253:0 252:0 252:16 8:0 8:16 1:0; do
  sudo cgset -r blkio.throttle.read_bps_device="${dev} 1048576" sandbox
  sudo cgset -r blkio.throttle.write_bps_device="${dev} 1048576" sandbox
done

# Deny access to devices
sudo cgset -r devices.deny=a sandbox
 
# Allow access to console, null, zero, random, unrandom
for d in "c 5:1" "c 1:3" "c 1:5" "c 1:8" "c 1:9"; do
  sudo cgset -r devices.allow="$d rw" sandbox
done

sudo cgdelete -g cpu,memory,blkio,devices,freezer:/isp

# Create cgroups
sudo cgcreate -g cpu,memory,blkio,devices,freezer:/isp
# Allows only 1ms every 100ms to simulate a slow system
sudo cgset -r cpu.cfs_period_us=100000 -r cpu.cfs_quota_us=1000 isp
# Set a limit of 2Gb
sudo cgset -r memory.limit_in_bytes=2G isp
# Limit block I/O to 1MB/s
for dev in 253:0 252:0 252:16 8:0 8:16 1:0; do
  sudo cgset -r blkio.throttle.read_bps_device="${dev} 1048576" isp
  sudo cgset -r blkio.throttle.write_bps_device="${dev} 1048576" isp
done
# Deny access to devices
sudo cgset -r devices.deny=a isp
# Allow access to console, null, zero, random, unrandom
for d in "c 5:1" "c 1:3" "c 1:5" "c 1:8" "c 1:9"; do
  sudo cgset -r devices.allow="$d rw" isp
done
 
# Create network namespace
#ip netns add sandbox
 
  prlimit --nofile=256 --nproc=512 --locks=32         \
# Join cgroup, netns and activate resources limit
sudo cgexec -g cpu,memory,blkio,devices,freezer:/isp       \
    ip netns exec isp                                 \
      unshare --mount --net --uts --ipc --pid --mount-proc=/proc --fork sh -c "
        mount -t tmpfs none /home
        mount -t tmpfs none /tmp
        mount -t tmpfs none /sys
        mount -t tmpfs none /var/log
        exec su -l ${SUDO_USER}
      "


sudo cgdelete -g cpu,memory,blkio,devices,freezer:/isp

sudo ip netns exec isp unshare -p -f --mount-proc=/proc chroot /var/cache/lxc/debian-bgp-ecmp/rootfs-stable-amd64/ /bin/bash

sudo cgcreate -g cpu,memory,blkio,devices,freezer:/isp
#sudo cgset -r cpu.cfs_period_us=100000 -r cpu.cfs_quota_us=1000 isp
sudo cgset -r memory.limit_in_bytes=500M isp

sudo cgset -r devices.deny=a isp
# Allow access to console, null, zero, random, unrandom
for d in "c 5:1" "c 1:3" "c 1:5" "c 1:8" "c 1:9"; do
  sudo cgset -r devices.allow="$d rw" isp
done
 
sudo cgexec -g cpu,memory,blkio,devices,freezer:/isp       \
  prlimit --nofile=256 --nproc=512 --locks=32         \
    ip netns exec isp                                 \
      unshare -i -m -u -C -p -f --mount-proc=/proc    \
        chroot /var/cache/lxc/debian-bgp-ecmp/rootfs-stable-amd64/ /bin/dash

sudo cgdelete -g cpu,memory,blkio,devices,freezer:/isp

sudo ip netns exec spine-a.z unshare -i -m -u -C -p -f --mount-proc=/proc chroot /var/cache/lxc/debian-bgp-ecmp/rootfs-stable-amd64/ /bin/dash

exit 0

ip link add veth0 type veth peer name veth1
ip link set veth1 netns blue

ip netns add blue
ip netns add red
ip link add veth0 netns blue type veth peer name veth1 netns red
ip netns exec blue ip link delete veth0

ip link add eth0 netns blue type veth peer name eth0 netns red
ip netns exec blue ip link delete eth0

# An easy container:
sudo ip netns exec isp chroot /var/cache/lxc/debian-bgp-ecmp/rootfs-stable-amd64/ bash
sudo nsenter --net=/var/run/netns/isp /bin/bash
https://blogs.rdoproject.org/2015/08/hands-on-linux-sandbox-with-namespaces-and-cgroups/
