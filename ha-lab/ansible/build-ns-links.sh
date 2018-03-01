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
  
exit 0

ip link add veth0 type veth peer name veth1
ip link set veth1 netns blue

ip netns add blue
ip netns add red
ip link add veth0 netns blue type veth peer name veth1 netns red
ip netns exec blue ip link delete veth0

ip link add eth0 netns blue type veth peer name eth0 netns red
ip netns exec blue ip link delete eth0
