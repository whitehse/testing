#!/bin/bash

for foo in `sudo ip netns list`; do   sudo ip netns delete $foo; done

rm hosts

HOSTS=`mktemp`
cat "../topology.dot" | sed 's/#.*//' | awk '{print $1}' | grep eth | awk -F':' '{print $1}' | sed 's/\"//g' | sort | uniq > $HOSTS
cat "../topology.dot" | sed 's/*.*//' | awk '{print $3}' | grep eth | awk -F':' '{print $1}' | sed 's/\"//g' | sort | uniq >> $HOSTS
for HOST in `cat $HOSTS | sort | uniq | grep -v host`
do
  sudo cgdelete -g cpu,memory,blkio,devices,freezer:/${HOST}
  sudo btrfs subvolume delete build.${HOST}
  echo "$HOST" >> hosts
done
rm $HOSTS
killall sleep
